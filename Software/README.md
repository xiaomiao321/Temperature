# 八通道 PT100 温度监控主机 (Final)

STM32F103C8T6 固件：实时采集 8 路 PT100 温度，任一通道超过阈值即报警，联动 4 路继电器
（警报 / 风机 / 断气阀 / 防火阀）。交付版带串口调试输出。

面向现场安装、使用与维护的完整说明（接线图、操作方法、故障排查）见 [`交付说明.pdf`](交付说明.pdf)；
本文件面向开发与维护。

## 硬件

- 主控：STM32F103C8T6，64KB Flash / 20KB RAM
- 温度：8 路 PT100 采集卡，Modbus 0x04 主动上报，经 RS485/TTL 接 UART2（PA2/PA3），只收不发
- 继电器：K1–K4（PB5 / PB6 / PA0 / PA1），报警吸合、正常释放，公共端接 24V
- 显示：两组 3 位共阳数码管，74HC595 驱动；面板 3 只指示灯，实装为两红一蓝，颜色不按常规对应——报警=大蓝灯（PB14），心跳/手动=小红灯（PB13/PB15）；引脚宏仍名 `LED_Red`/`LED_Yellow`/`LED_Blue`，与实焊颜色不一致
- 按键：设置 4 键（PB3/4/0/1，低有效）+ 手动 / 复位（PA12 / PA15，高有效）
- 调试：UART1（PA9 / PA10）@115200 8N1
- 可靠性：独立看门狗，超时约 1.6s

## 分层架构

依赖只自上而下。业务层不直接碰寄存器，操作硬件一律走驱动层 API。

```
应用   Core/Src/main.c        初始化 + 主循环调度
业务   App/logic/             temperature 帧解析、报警/继电器/LED、防火阀状态机
       (不碰硬件)             temp_setting 阈值、display 显示、report 日志格式化
驱动   App/driver/            hc595 数码管、key 按键   (relay/led/flash/uart 待抽取)
BSP    Core/ + Lib/           board_config.h 引脚语义、CubeMX(gpio/usart/dma/iwdg/it)
                              STM32 HAL、MultiButton
```

CubeMX 再生成只影响 `Core/`，不会动 `App/` 下的应用代码。当前 `hc595`、`key` 已是驱动层；
`relay`、`led`、`flash_store`、`uart_rx`、`debug_uart` 后续可继续从业务层抽出。

## 目录

| 路径 | 说明 |
|---|---|
| `App/logic/` | 业务逻辑：temperature、temp_setting、display、report |
| `App/driver/` | 设备驱动：hc595、key |
| `Core/Inc/board_config.h` | 引脚语义映射 + 可调参数，单一事实来源 |
| `Core/Src/`、`Core/Inc/` | CubeMX 生成的 BSP：main、gpio、usart、dma、iwdg、中断 |
| `Lib/MultiButton/` | 按键库 |
| `Ref/` | 参考资料，厂家例程等 |
| `交付说明.tex` / `.pdf` | 交付说明书：接线、操作、维护 |

## 编译与烧录

工具链：STM32CubeMX + CMake + Ninja + arm-none-eabi-gcc（工具链文件
`cmake/gcc-arm-none-eabi.cmake`）。

```sh
cmake --preset Debug                        # 首次配置，输出到 build/Debug
cmake --build build/Debug --target Final    # 增量编译
```

产物 `build/Debug/Final.elf` / `.hex` / `.bin`，用 ST-Link 烧录 `.hex` 或 `.elf`。

## 配置

可调参数集中在 `Core/Inc/board_config.h`：继电器功能映射、阈值默认值、报警抑制期、防火阀方案。
改板或量产只看这一个文件。

温度阈值分两组，阈值1 管 CH1–CH7、阈值2 管 CH8；出厂默认为占位值，现场用设置键标定后写入
Flash，掉电保存。

## 串口调试

UART1 @115200 8N1。上电打印横幅（含编译时间戳，便于核对固件版本），运行时每 1s 一行：

```
t=123.456 T1=25.3 ... T8=ERR TH1=100 TH2=100 ALM=0 MAN=0 LAT=0 BM=0 BR=0 K=0000
```

字段含义见《交付说明》电脑串口监控一节。没接采集卡也照打，无效通道显示 ERR，插上串口即可
确认程序在跑。

## 修改指引

| 想改的 | 看哪里 |
|---|---|
| 阈值 / 抑制期 / 防火阀方案 | `Core/Inc/board_config.h` |
| 报警联动、防火阀时序 | `App/logic/temperature.c` |
| 显示内容与节奏 | `App/logic/display.c` |
| 段码字形 | `App/driver/hc595.c` |
| 串口日志字段 | `App/logic/report.c` |
| 按键有效电平 | `App/driver/key.c` |
