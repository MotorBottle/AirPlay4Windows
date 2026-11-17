# Fork changes / 分支改动

## 来源 / Provenance

| 项目 / Project | 来源与职责 / Source and role |
|---|---|
| AirPlay Bridge | [RiverFlowsInUUU/AirPlay-Bridge](https://github.com/RiverFlowsInUUU/AirPlay-Bridge)，原 Windows 应用 / original Windows application |
| 基线 / Base | 上游 v1.5.58，`fc6ef4150adf335c19bed75870fa90c1bbf0e2b3` |
| AirPlay4Windows | [MotorBottle/AirPlay4Windows](https://github.com/MotorBottle/AirPlay4Windows)，UI 与体验分支 / UI and user-experience fork |
| 协议引擎 / Protocol engine | [Andreas Wendorf / airplay2-sender-cpp](https://github.com/akustikrausch/airplay2-sender-cpp)；以子模块和补丁保留 / retained as a submodule with patches |

上游 Git 历史与作者信息完整保留，没有重新初始化仓库或将上游贡献压成一次提交。
Upstream history and authorship are preserved; this is not a reinitialized or squashed copy.

## 改动范围 / Scope

主要工作为托盘面板和交互体验，并有配套音频调整；不将上游应用或协议引擎称为本分支原创。
The focus is the tray panel and interaction experience, with supporting audio changes; the upstream application and protocol engine are credited to their authors.

| 范围 / Area | 改动 / Changes |
|---|---|
| 面板 / Panel | 单层深色设备卡片、直接音量控制、拖动、任务栏隐藏、点击外部收起 / dark device cards, inline volume, dragging, no taskbar button, outside-click dismissal |
| 操作 / Interaction | 推流开关、统一底部图标、设置窗口可见性与消息循环 / streaming switch, consistent footer icons, visible settings and responsive message loops |
| 音频 / Audio | 默认进程环回、暂停和退出恢复本机声音、无声时静音误判修复 / default process loopback, local-audio restoration, false mute-incompatibility fix |
| 缓冲与音量 / Buffer and volume | 请求缓冲范围与协议参数联动、采集轮询、接收端音量控制与诊断 / requested-buffer propagation, capture polling, receiver-volume control and diagnostics |

## 可追溯记录 / Traceability

| Commit | 内容 / Contents |
|---|---|
| `16bc493` | 面板及采集、缓冲、音量和静音交互 / panel, capture, buffer, volume and mute interactions |
| `eb771ec` | 设置窗口可见性和面板响应 / settings visibility and panel responsiveness |
| `c06fd41` | 底部图标统一及验收记录 / footer alignment and verification notes |
| `6515e3e` | 外部收起与静音误判修复 / outside dismissal and false mute detection |

各版本用户可见变化见 [CHANGELOG](../CHANGELOG.md)；更早条目属于上游历史。
See [CHANGELOG](../CHANGELOG.md) for versioned user-facing changes; older entries are upstream history.

## 维护范围 / Maintenance scope

设备发现、多网卡、地址变化后的身份核对等不足见 [README](../README.zh-CN.md#当前不足)。目前不计划主动完善这些功能；欢迎针对明确问题提交 PR，或自行 fork。
Discovery, multiple adapters and identity checks after address changes remain limited; see [README](../README.md#current-limitations). Further work is not currently planned. Focused PRs and independent forks are welcome.

截图由当前面板代码的独立无声预览生成，不含实际网络设备资料。主要验证环境为 Windows 与 HomePod mini；缓冲数值不是端到端延迟实测值。
The screenshot comes from a silent preview using the actual panel code and example device data. Validation has focused on Windows and HomePod mini; buffer values are not measured end-to-end latency.

许可证见 [LICENSE](../LICENSE)，署名与第三方声明见 [NOTICE](../NOTICE)。MotorBottle 维护本分支，开发中使用了 Codex 辅助。
See [LICENSE](../LICENSE) and [NOTICE](../NOTICE). MotorBottle maintains this fork, with development assistance from Codex.
