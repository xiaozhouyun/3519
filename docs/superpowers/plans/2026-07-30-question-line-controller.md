# Per-Question Line Controller Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Store the second and third question follow-line settings in independent `LineController_t` objects, with the third question initially copied from the second and selected for current Bluetooth tuning.

**Architecture:** Extend `LineController_t` with the base speed, replace the single global controller with two question-specific controllers plus one active-controller pointer, and let the follow-line update use the active controller. Bluetooth selects question `2` or `3` and adjusts only the active controller.

**Tech Stack:** C for MSPM0G3519, FreeRTOS, PowerShell source-contract checks, Keil project.

## Global Constraints

- Keep the current second-question defaults exactly `Kp=5.20f`, `Ki=0.0f`, `Kd=0.002f`, `base_speed=880`.
- Initialize the third question by copying the initialized second-question controller.
- Select question 3 after initialization so current tuning applies to question 3.
- Do not change the PID formula, grayscale interpretation, motor PID ownership, or motor driver interface.
- Do not write parameters to Flash; a reboot restores third-question defaults from question 2.
- Keep the change limited to parameter ownership, question selection, and their static verification.

---

### Task 1: Add the source contract for per-question control

**Files:**
- Create: `tests/follow_line_question_controller_contract.ps1`
- Modify: `tests/pid_parameter_source_contract.ps1`

**Interfaces:**
- Consumes: source paths `core/inc/follow_line.h`, `core/src/follow_line.c`, `core/src/blue.c`, and `main.c`.
- Produces: a static test that proves question 2 and 3 have independent controllers, question 3 starts from question 2, Bluetooth chooses the active question, and the chassis task does not pass literal `880`.

- [ ] **Step 1: Write the failing test**

Create `tests/follow_line_question_controller_contract.ps1` with these checks:

```powershell
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$header = Get-Content -Raw (Join-Path $root 'core\inc\follow_line.h')
$source = Get-Content -Raw (Join-Path $root 'core\src\follow_line.c')
$blue = Get-Content -Raw (Join-Path $root 'core\src\blue.c')
$main = Get-Content -Raw (Join-Path $root 'main.c')

if ($header -notmatch 'int16_t\s+base_speed\s*;') { throw 'LineController_t must own base_speed.' }
if ($source -notmatch 'LineController_t\s+g_question2_line_controller\s*;') { throw 'Question 2 controller is missing.' }
if ($source -notmatch 'LineController_t\s+g_question3_line_controller\s*;') { throw 'Question 3 controller is missing.' }
if ($source -notmatch 'g_question3_line_controller\s*=\s*g_question2_line_controller\s*;') { throw 'Question 3 must start from question 2.' }
if ($main -notmatch 'FollowLine_Select_Question\s*\(\s*3U\s*\)') { throw 'Question 3 must be selected after initialization.' }
if ($blue -notmatch "case\s+'2'\s*:\s*FollowLine_Select_Question\s*\(\s*2U\s*\)") { throw 'Bluetooth command 2 must select question 2.' }
if ($blue -notmatch "case\s+'3'\s*:\s*FollowLine_Select_Question\s*\(\s*3U\s*\)") { throw 'Bluetooth command 3 must select question 3.' }
if ($blue -notmatch 'g_active_line_controller->kp') { throw 'Bluetooth PID adjustment must target the active controller.' }
if ($main -match 'FollowLine_Update\s*\([^\)]*,\s*880\s*\)') { throw 'ChassisTask must not hard-code base speed 880.' }
Write-Output 'Question line-controller contract passed.'
```

Replace the single-controller expectation in `tests/pid_parameter_source_contract.ps1` with a check that `core/src/follow_line.c` declares both question controllers without initializer lists.

- [ ] **Step 2: Run test to verify it fails**

Run: `powershell -ExecutionPolicy Bypass -File .\tests\follow_line_question_controller_contract.ps1`

Expected: FAIL with `LineController_t must own base_speed.`

- [ ] **Step 3: Commit the failing test**

```powershell
git add tests/follow_line_question_controller_contract.ps1 tests/pid_parameter_source_contract.ps1
git commit -m "test: define per-question line controller contract"
```

### Task 2: Refactor follow-line controller ownership

**Files:**
- Modify: `core/inc/follow_line.h`
- Modify: `core/src/follow_line.c`

**Interfaces:**
- Consumes: existing `LineController_t`, `FollowLine_Reset()`, `FollowLine_PID_Compute()`, and `MG513XGMR_Set_Speed()`.
- Produces: `g_question2_line_controller`, `g_question3_line_controller`, `g_active_line_controller`, `FollowLine_Select_Question(uint8_t)`, `FollowLine_Get_Active_Question(void)`, and `FollowLine_Update(Grayscale_Sensor_t *sensor)`.

- [ ] **Step 1: Implement the minimal header interface**

Add `int16_t base_speed` to `LineController_t`; declare the two question controllers and `LineController_t *g_active_line_controller`; use these declarations:

```c
void FollowLine_Init(LineController_t *p_ctrl, float kp, float ki, float kd,
                     int16_t base_speed);
bool FollowLine_Select_Question(uint8_t question);
uint8_t FollowLine_Get_Active_Question(void);
float FollowLine_Calc_Error(LineController_t *p_ctrl, Grayscale_Sensor_t *sensor);
float FollowLine_Update(Grayscale_Sensor_t *sensor);
```

- [ ] **Step 2: Implement the minimal source behavior**

Declare two zero-initialized controller objects and an active pointer. `FollowLine_Init()` stores `base_speed`. `FollowLine_Select_Question()` maps only `2U` and `3U` to their matching object, makes it active, clears its PID history through `FollowLine_Reset()`, and returns `true`; all other values return `false` without changing selection. `FollowLine_Get_Active_Question()` returns `2U` or `3U` based on the active pointer.

Make `FollowLine_Calc_Error()` use `p_ctrl->last_error`. `FollowLine_Update()` obtains the active controller and calls:

```c
MG513XGMR_Set_Speed(MG513XGMR_LEFT, p_ctrl->base_speed + g_turn_output);
MG513XGMR_Set_Speed(MG513XGMR_RIGHT, p_ctrl->base_speed - g_turn_output);
```

- [ ] **Step 3: Run the controller contracts**

Run:

```powershell
powershell -ExecutionPolicy Bypass -File .\tests\pid_parameter_source_contract.ps1
powershell -ExecutionPolicy Bypass -File .\tests\follow_line_speed_mode_contract.ps1
```

Expected: both PASS. The question-controller test remains RED until Task 3.

### Task 3: Initialize both questions and scope Bluetooth tuning

**Files:**
- Modify: `main.c`
- Modify: `core/src/blue.c`
- Modify: `core/inc/blue.h`

**Interfaces:**
- Consumes: Task 2’s question controllers and question-selection functions.
- Produces: question 2 defaults, a copied question 3 default, default selection of question 3, and Bluetooth adjustment/query output scoped to the selected question.

- [ ] **Step 1: Initialize the question controllers in `main.c`**

Replace the one `FollowLine_Init()` call with:

```c
FollowLine_Init(&g_question2_line_controller, 5.20f, 0.0f, 0.002f, 880);
g_question3_line_controller = g_question2_line_controller;
FollowLine_Select_Question(3U);
```

Replace the chassis call with:

```c
FollowLine_Update(&g_grayscale_sensor);
```

- [ ] **Step 2: Scope Bluetooth commands in `blue.c`**

Make `Bluetooth_Send_PID_Params()` print `FollowLine_Get_Active_Question()`, `g_active_line_controller->kp`, `ki`, `kd`, and `base_speed`. Add cases `2` and `3` that call `FollowLine_Select_Question(2U)` and `FollowLine_Select_Question(3U)` then report the selected parameter set. Change all PID increment/decrement cases to use `g_active_line_controller->kp`, `ki`, and `kd`, guarded by a non-null active pointer. Remove the old `3` speed-grade behavior because `3` is now the third-question selection command. Update header comments to describe per-question PID tuning.

- [ ] **Step 3: Run all focused static checks**

Run:

```powershell
powershell -ExecutionPolicy Bypass -File .\tests\follow_line_question_controller_contract.ps1
powershell -ExecutionPolicy Bypass -File .\tests\pid_parameter_source_contract.ps1
powershell -ExecutionPolicy Bypass -File .\tests\follow_line_speed_mode_contract.ps1
```

Expected: all PASS.

- [ ] **Step 4: Inspect the final diff and commit**

Run:

```powershell
git diff --check
git diff -- main.c core/inc/follow_line.h core/src/follow_line.c core/inc/blue.h core/src/blue.c tests/follow_line_question_controller_contract.ps1 tests/pid_parameter_source_contract.ps1
```

Expected: only parameter-ownership, selection, Bluetooth, and focused-test changes. Commit only these files:

```powershell
git add main.c core/inc/follow_line.h core/src/follow_line.c core/inc/blue.h core/src/blue.c tests/follow_line_question_controller_contract.ps1 tests/pid_parameter_source_contract.ps1
git commit -m "feat: separate follow-line parameters by question"
```

### Task 4: Keil and board verification

**Files:**
- Verify: active Keil target and the source files changed above.

**Interfaces:**
- Consumes: a successful source-contract result from Task 3.
- Produces: user-provided Keil build log and board behavior evidence.

- [ ] **Step 1: Build in Keil**

Open the active Keil target and rebuild it. Confirm the build log reports `0 Error(s)` and that its `.axf` timestamp is fresh.

- [ ] **Step 2: Verify on the board**

With the Bluetooth terminal connected at the configured UART1 rate, send `Q`, `P`, `Q`, `2`, `Q`, `3`, and `Q`. Confirm `P` changes only the third-question value, switching to `2` reports the unchanged second-question values, and switching back to `3` restores the adjusted third-question values.

