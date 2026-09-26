# STM32 FreeRTOS

本 Repository 為使用 **STM32F407 + FreeRTOS** 進行 RTOS 與 Embedded System 練習的專案集合。

從基本 Task Scheduling 開始，逐步實作 Task Priority、Task Notification、Semaphore、ISR 與 UART CLI，理解 RTOS 中 Multitasking、Synchronization 與 Event-driven Design。

---

## 開發環境

- MCU：STM32F407
- Board：STM32F407 Discovery
- IDE：STM32CubeIDE
- Language：C
- RTOS：FreeRTOS

---

## 主要練習內容

- Task Creation
- Task Scheduling
- Task Priority
- Task Notification
- Queue
- Binary Semaphore
- Interrupt / ISR
- UART Communication
- CLI Command Parsing
- Task Synchronization

---

## Task Scheduling

使用 `xTaskCreate()` 建立多個 Task，交由 FreeRTOS Scheduler 管理。

```text
        FreeRTOS Scheduler
                │
        ┌───────┼───────┐
        ▼       ▼       ▼
      Task A  Task B  Task C
```

搭配：

- `vTaskDelay()`
- Task Priority
- Task State
- Preemptive Scheduling

理解 Task 在：

```text
Ready
  ↓
Running
  ↓
Blocked
```

等不同狀態間的切換。

---

## Task Priority

透過設定不同 Task Priority，觀察 FreeRTOS Scheduler 如何選擇下一個執行的 Task。

```text
High Priority Task
        │
        ▼
Medium Priority Task
        │
        ▼
Low Priority Task
```

藉此理解：

- Preemption
- Priority Scheduling
- Context Switching

---

## Task Notification

使用 Task Notification 進行 Task 之間的 Event Signaling。

```text
Task A
  │
  │ xTaskNotify()
  ▼
Task B
```

Task Notification 不需要額外建立 Queue / Semaphore Object，可作為較 Lightweight 的 Task Communication 機制。

---

## Queue

使用 Queue 在不同 Task 之間傳遞資料。

```text
Producer Task
      │
      │ xQueueSend()
      ▼
    Queue
      │
      │ xQueueReceive()
      ▼
Consumer Task
```

Queue 可避免不同 Task 直接操作相同資料造成同步問題。

---

## Binary Semaphore

使用 Binary Semaphore 實作 Task Synchronization，以及 ISR 與 Task 間的 Event Notification。

```text
Button / Event
      │
      ▼
     ISR
      │
      ▼
Binary Semaphore
      │
      ▼
 Worker Task
```

將較多的處理工作交由 Task 執行，使 ISR 保持簡短。

---

## Interrupt + FreeRTOS

實作 External Interrupt 並搭配 FreeRTOS ISR-safe API。

主要概念：

- EXTI
- ISR Context
- ISR-safe FreeRTOS API
- Task Wake-up
- Context Switching

典型架構：

```text
Hardware Event
      │
      ▼
     ISR
      │
      │ Notify / Give
      ▼
    Task
      │
      ▼
Process Event
```

將系統由 Polling Design 改為 Event-driven Design。

---

## UART CLI

`FreeRTOS_CLI` 實作 UART-based Command Line Interface。

### UART Configuration

```text
Baud Rate : 115200
Data Bits : 8
Parity    : None
Stop Bits : 1
```

CLI Task 負責：

1. 接收 UART Character
2. 建立 Command Buffer
3. 處理 Enter / Backspace
4. Parse Command
5. 執行對應功能

目前包含：

- `clear`：清除 Terminal
- `states`：顯示 CLI 執行統計
- Character Echo
- Input Buffer Handling

---

## CLI Architecture

```text
PC Terminal
     │
     │ UART
     ▼
   USART2
     │
     ▼
  CLI Task
     │
     ▼
Command Parser
     │
     ▼
Command Execution
```

同時由 FreeRTOS Scheduler 管理不同 Task：

```text
FreeRTOS Scheduler
        │
        ├── CLI Task
        │
        └── Monitor Task
```

---

## Repository Projects

Repository 中包含多個獨立 FreeRTOS 練習專案：

```text
FreeRTOS/
│
├── 002LED_Task
├── 005LED_Task_Notify
├── 006LED_Button_ISR
├── 007LED_Priority_Swap
├── Binary_semaphore_tasks
├── FreeRTOS_Binary_Semaphore
├── STM32_FreeRTOS_Bin_Sema_Tasks
├── FreeRTOS_CLI
└── FreeRTOS_Mini_Command_Line
```

各 Project 用於測試不同 FreeRTOS 機制，從基本 Task 控制逐步延伸到 Synchronization 與 CLI Application。

---

## 實作重點

- Task Scheduling
- Preemptive Multitasking
- Task Priority
- Context Switching
- Task Notification
- Queue
- Semaphore
- UART Communication

- DMA UART
- Queue-based UART RX
- SEGGER SystemView Runtime Analysis
