#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "quick_panel.h"
#include "i18n.h"
#include <windowsx.h>
#include <objidl.h>
#include <gdiplus.h>
#include <dwmapi.h>
#include <algorithm>
#include <memory>
#include <utility>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "dwmapi.lib")

namespace fxchain {
namespace {
using namespace Gdiplus;
constexpr int kWidth = 440;
constexpr int kPad = 22;
const Color kBackground(255, 29, 30, 32), kCard(255, 43, 45, 48);
const Color kAccent(255, 91, 201, 244), kText(255, 244, 245, 247);
const Color kMuted(255, 163, 168, 176), kInk(255, 14, 36, 48);
struct Panel {
    HWND hwnd = nullptr;
    HWND owner = nullptr;
    int volume = 0, action = 0, liveVolume = -1, height = 0;
    bool close = false, dragging = false, moving = false;
    POINT dragStart{}, windowStart{};
    float scale = 1;
    ULONGLONG lastVolumeAt = 0;
    QuickPanelStatus status;
    std::vector<TrayMenuItem> devices;
    std::function<QuickPanelStatus()> getStatus;
    std::function<void(int)> command;
};
std::unique_ptr<Panel> g_panel;
bool g_reopenAtSamePosition = false;
POINT g_reopenPosition{};
std::wstring wide(const std::string& value) {
    if (value.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, value.data(), int(value.size()), nullptr, 0);
    std::wstring result(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, value.data(), int(value.size()), result.data(), n);
    return result;
}
void rounded(Graphics& g, RectF r, float radius, Color fill, Color border = Color(0,0,0,0)) {
    GraphicsPath path;
    float d = radius * 2;
    path.AddArc(r.X, r.Y, d, d, 180, 90);
    path.AddArc(r.GetRight()-d, r.Y, d, d, 270, 90);
    path.AddArc(r.GetRight()-d, r.GetBottom()-d, d, d, 0, 90);
    path.AddArc(r.X, r.GetBottom()-d, d, d, 90, 90);
    path.CloseFigure();
    SolidBrush b(fill);g.FillPath(&b, &path);
    if (border.GetA()) { Pen p(border, 1); g.DrawPath(&p, &path); }
}
void label(Graphics& g, const std::wstring& text, RectF bounds, float size, Color color,
           bool bold = false, StringAlignment align = StringAlignmentNear) {
    Font font(L"Microsoft YaHei UI", size, bold ? FontStyleBold : FontStyleRegular, UnitPixel);
    StringFormat f;f.SetAlignment(align);f.SetLineAlignment(StringAlignmentCenter);
    f.SetFormatFlags(StringFormatFlagsNoWrap);f.SetTrimming(StringTrimmingEllipsisCharacter);
    SolidBrush b(color);g.DrawString(text.c_str(), int(text.size()), &font, bounds, &f, &b);
}
void glyph(Graphics& g, wchar_t code, RectF bounds, float size, Color color) {
    Font font(L"Segoe MDL2 Assets", size, FontStyleRegular, UnitPixel);
    StringFormat f;f.SetAlignment(StringAlignmentCenter);f.SetLineAlignment(StringAlignmentCenter);
    SolidBrush b(color);g.DrawString(&code, 1, &font, bounds, &f, &b);
}
RectF deviceRect(size_t i) { return RectF(float(kPad + (i%2)*204), float(155+(i/2)*106), 192, 94); }
void paint(Panel& p, HDC dc) {
    RECT client{};GetClientRect(p.hwnd, &client);
    Bitmap buffer(client.right, client.bottom, PixelFormat32bppPARGB);
    Graphics g(&buffer);g.Clear(kBackground);g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);g.ScaleTransform(p.scale,p.scale);
    const bool active = p.status.connected && !p.status.paused;
    SolidBrush dot(active ? Color(255,105,216,104) : (p.status.running ? kAccent : kMuted));
    g.FillEllipse(&dot, 23.0f, 35.0f, 10.0f, 10.0f);
    label(g, wide(p.status.state), {45,21,284,31}, 20, kText, true);
    label(g, wide(p.status.device.empty()?i18n::tr("选择扬声器开始播放"):p.status.device), {45,51,295,28}, 12, kMuted);
    rounded(g, {360,30,56,28},14, active?kAccent:Color(255,77,81,87));
    SolidBrush knob(active?kInk:kText);g.FillEllipse(&knob, active?390.0f:364.0f,34.0f,20.0f,20.0f);
    glyph(g, p.volume?0xE767:0xE74F,{20,100,26,26},21,kText);
    rounded(g,{59,111,293,5},2.5f,Color(255,105,109,116));
    float thumb = 59 + 293 * p.volume / 100.0f;
    if(p.volume)rounded(g,{59,111,thumb-59,5},2.5f,kAccent);
    SolidBrush halo(Color(55,91,201,244));g.FillEllipse(&halo,thumb-13,100.5f,26.0f,26.0f);
    SolidBrush blue(kAccent);g.FillEllipse(&blue,thumb-7,106.5f,14.0f,14.0f);
    label(g,std::to_wstring(p.volume)+L"%",{365,99,53,28},18,kText,false,StringAlignmentFar);
    for(size_t i=0;i<p.devices.size();++i) {
        auto r=deviceRect(i);auto& d=p.devices[i];
        bool selected = p.status.running && d.label.find(p.status.device)!=std::string::npos && !p.status.device.empty();
        rounded(g,r,9,selected?kAccent:kCard,selected?kAccent:Color(255,62,65,70));
        Color fg=selected?kInk:kText, secondary=selected?Color(255,28,69,85):kMuted;
        glyph(g,0xE7F5,{r.X+12,r.Y+10,27,24},23,fg);
        if(selected)glyph(g,p.status.paused?0xE768:0xE769,{r.GetRight()-36,r.Y+12,23,23},15,fg);
        std::string name=d.label;
        const auto ip=name.find("  ");if(ip!=std::string::npos)name.resize(ip);
        label(g,wide(name),{r.X+13,r.Y+39,r.Width-26,26},14,fg,true);
        label(g,selected?i18n::trw(p.status.paused?"已暂停":"已连接"):i18n::trw("点击连接"),{r.X+13,r.Y+66,r.Width-26,19},11,secondary);
    }
    if(p.devices.empty()) {
        rounded(g,{22,155,396,94},9,kCard,Color(255,62,65,70));
        label(g,i18n::trw("连接上次的扬声器"),{38,169,360,30},16,kText,true);
        label(g,i18n::trw("或点击右下角重新扫描"),{38,201,360,24},12,kMuted);
    }
    float foot=float(p.height-64);SolidBrush footer(Color(255,36,38,41));g.FillRectangle(&footer,0.0f,foot,440.0f,64.0f);
    Pen line(Color(255,57,60,65));g.DrawLine(&line,0.0f,foot,440.0f,foot);
    rounded(g,{22,foot+16,104,32},6,Color(255,49,52,56),Color(255,65,69,74));
    label(g,i18n::trw("延迟")+L" "+std::to_wstring(p.status.latency)+L" ms",{22,foot+16,104,32},12,kText,false,StringAlignmentCenter);
    label(g,i18n::trw("系统声音"),{142,foot+16,80,32},12,kMuted);
    rounded(g,{238,foot+16,66,32},6,Color(255,49,52,56),Color(255,65,69,74));
    label(g,i18n::trw("退出"),{238,foot+16,66,32},12,kText,false,StringAlignmentCenter);
    glyph(g,0xE72C,{326,foot+16,36,32},18,kMuted);
    glyph(g,0xE713,{378,foot+16,36,32},19,kText);
    Graphics screen(dc);screen.DrawImage(&buffer,0,0);
}
void slide(Panel& p, int x) {
    int v=std::clamp(int((x/p.scale-59)*100/293+0.5f),0,100);
    if(v!=p.volume){p.volume=v;p.liveVolume=v;InvalidateRect(p.hwnd,nullptr,FALSE);}
}
LRESULT CALLBACK panelProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp) {
    auto* p=reinterpret_cast<Panel*>(GetWindowLongPtrW(hwnd,GWLP_USERDATA));
    if(msg==WM_NCCREATE) {
        p=static_cast<Panel*>(reinterpret_cast<CREATESTRUCTW*>(lp)->lpCreateParams);
        SetWindowLongPtrW(hwnd,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(p));p->hwnd=hwnd;
    }
    if(!p)return DefWindowProcW(hwnd,msg,wp,lp);
    switch(msg) {
    case WM_ERASEBKGND:return 1;
    case WM_PAINT:{PAINTSTRUCT ps;HDC dc=BeginPaint(hwnd,&ps);paint(*p,dc);EndPaint(hwnd,&ps);return 0;}
    case WM_CLOSE:p->close=true;return 0;
    case WM_KEYDOWN:
        if(wp==VK_ESCAPE)p->close=true;
        else if(wp==VK_LEFT || wp==VK_DOWN){p->volume=std::max(0,p->volume-1);p->liveVolume=p->volume;}
        else if(wp==VK_RIGHT || wp==VK_UP){p->volume=std::min(100,p->volume+1);p->liveVolume=p->volume;}
        else if(wp==VK_SPACE)p->action=p->status.running?10900:101;
        InvalidateRect(hwnd,nullptr,FALSE);return 0;
    case WM_LBUTTONDOWN:{
        float x=GET_X_LPARAM(lp)/p->scale,y=GET_Y_LPARAM(lp)/p->scale;
        if(y>=94 && y<=135 && x>=48 && x<=363){p->dragging=true;SetCapture(hwnd);slide(*p,GET_X_LPARAM(lp));}
        else if(x>=350 && y>=20 && y<=72)p->action=p->status.running?10900:101;
        else if(y<85) {
            p->moving=true;
            const DWORD pos=GetMessagePos();
            p->dragStart={GET_X_LPARAM(pos),GET_Y_LPARAM(pos)};
            RECT rect{};GetWindowRect(hwnd,&rect);p->windowStart={rect.left,rect.top};
            SetCapture(hwnd);
        }
        else if(y>=p->height-60) {
            if(x<132)p->action=106;
            else if(x>=238 && x<=304)p->action=104;
            else if(x>=315 && x<372)p->action=113;
            else if(x>=372)p->action=10902;
        } else if(p->devices.empty() && y>=155 && y<=249)p->action=101;
        else for(size_t i=0;i<p->devices.size();++i)if(deviceRect(i).Contains(x,y)){p->action=p->devices[i].id;break;}
        return 0;
    }
    case WM_MOUSEMOVE:
        if(p->dragging)slide(*p,GET_X_LPARAM(lp));
        if(p->moving) {
            const DWORD pos=GetMessagePos();
            POINT cursor{GET_X_LPARAM(pos),GET_Y_LPARAM(pos)};
            SetWindowPos(hwnd,nullptr,p->windowStart.x+cursor.x-p->dragStart.x,
                         p->windowStart.y+cursor.y-p->dragStart.y,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        }
        return 0;
    case WM_LBUTTONUP:
        if(p->dragging)slide(*p,GET_X_LPARAM(lp));
        if(p->moving) {
            RECT rect{};GetWindowRect(hwnd,&rect);
            g_reopenPosition={rect.left,rect.top};g_reopenAtSamePosition=true;
        }
        if(p->dragging || p->moving){p->dragging=false;p->moving=false;ReleaseCapture();}
        return 0;
    case WM_CAPTURECHANGED:p->dragging=false;p->moving=false;return 0;
    case WM_TIMER:
        if(p->getStatus){p->status=p->getStatus();InvalidateRect(hwnd,nullptr,FALSE);}return 0;
    }
    return DefWindowProcW(hwnd,msg,wp,lp);
}
}
void closeQuickPanel() {
    if(!g_panel)return;
    DestroyWindow(g_panel->hwnd);g_panel.reset();
}
void toggleQuickPanel(HWND owner,const std::vector<TrayMenuItem>& menu,int volume,
                      const std::function<QuickPanelStatus()>& status,const std::function<void(int)>& command) {
    if(g_panel){closeQuickPanel();return;}
    static ULONG_PTR token=0;
    if(!token){GdiplusStartupInput input;GdiplusStartup(&token,&input,nullptr);}
    auto p=std::make_unique<Panel>();p->owner=owner;p->volume=volume;p->getStatus=status;p->command=command;p->status=status();
    for(const auto& group:menu)if(group.id==122)for(const auto& row:group.children)
        if(row.id>=1400 && row.id<1600)p->devices.push_back(row);
    POINT pt{};GetCursorPos(&pt);MONITORINFO mi{sizeof(mi)};GetMonitorInfoW(MonitorFromPoint(pt,MONITOR_DEFAULTTONEAREST),&mi);
    p->scale=GetDpiForWindow(owner)/96.0f;
    p->height=155+int(std::max<size_t>(1,(p->devices.size()+1)/2))*106+18+64;
    int w=int(kWidth*p->scale),h=int(p->height*p->scale);
    int x=std::clamp(int(pt.x)-w/2,int(mi.rcWork.left+8),int(mi.rcWork.right-w-8));
    int y=std::clamp(int(pt.y)-h-12,int(mi.rcWork.top+8),int(mi.rcWork.bottom-h-8));
    if (g_reopenAtSamePosition) {
        MONITORINFO previous{sizeof(previous)};
        GetMonitorInfoW(MonitorFromPoint(g_reopenPosition,MONITOR_DEFAULTTONEAREST),&previous);
        x=std::clamp(int(g_reopenPosition.x),int(previous.rcWork.left+8),
                     int(std::max(previous.rcWork.left+8,previous.rcWork.right-w-8)));
        y=std::clamp(int(g_reopenPosition.y),int(previous.rcWork.top+8),
                     int(std::max(previous.rcWork.top+8,previous.rcWork.bottom-h-8)));
    }
    WNDCLASSW wc{};wc.lpfnWndProc=panelProc;wc.hInstance=GetModuleHandleW(nullptr);wc.lpszClassName=L"HomePodQuickPanel";
    wc.hCursor=LoadCursorW(nullptr,MAKEINTRESOURCEW(32512));wc.style=CS_DROPSHADOW;RegisterClassW(&wc);
    // 独立顶层 popup，无标题栏；正常使用不占任务栏，--panel-inspect 仅供视觉验收。
    DWORD ex=WS_EX_TOPMOST|(wcsstr(GetCommandLineW(),L"--panel-inspect")?WS_EX_APPWINDOW:WS_EX_TOOLWINDOW);
    HWND hwnd=CreateWindowExW(ex,wc.lpszClassName,L"HomePod · AirPlay Bridge",WS_POPUP|WS_BORDER,x,y,w,h,nullptr,nullptr,wc.hInstance,p.get());
    if(!hwnd)return;
    DWORD corner=2;DwmSetWindowAttribute(hwnd,33,&corner,sizeof(corner));
    BOOL dark=TRUE;DwmSetWindowAttribute(hwnd,20,&dark,sizeof(dark));
    SetWindowRgn(hwnd,CreateRoundRectRgn(0,0,w+1,h+1,int(20*p->scale),int(20*p->scale)),TRUE);
    SetTimer(hwnd,1,250,nullptr);g_panel=std::move(p);
    ShowWindow(hwnd,SW_SHOWNORMAL);SetForegroundWindow(hwnd);
}
void pumpQuickPanel() {
    if(!g_panel)return;
    if(g_panel->close){closeQuickPanel();return;}
    auto& p=*g_panel;
    if(p.liveVolume>=0 && (!p.dragging || GetTickCount64()-p.lastVolumeAt>=70)) {
        int v=std::exchange(p.liveVolume,-1);p.lastVolumeAt=GetTickCount64();p.command(10000+v);
    }
    if(p.action) {
        int action=std::exchange(p.action,0);auto callback=p.command;HWND owner=p.owner;
        if (action==10900) {
            callback(action);p.status=p.getStatus();InvalidateRect(p.hwnd,nullptr,FALSE);return;
        }
        // 从消息分发外执行命令，避免销毁窗口时回调重入。
        RECT position{};GetWindowRect(p.hwnd,&position);
        g_reopenPosition={position.left,position.top};g_reopenAtSamePosition=true;
        closeQuickPanel();callback(action);
        if(action!=104)PostMessageW(owner,WM_APP+1,0,WM_LBUTTONUP);
    }
}
bool quickPanelMessage(MSG& msg) {
    return false; // 自绘面板的键盘事件交由 panelProc 处理。
}
}
