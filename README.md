# homeworkc

## Build & Quick Tests

### Build

```bash
# Linux/macOS
gcc -O2 -std=c11 -Wall -Wextra -o hw-01 hw-01.c
```

```powershell
# Windows (PowerShell)
gcc -O2 -std=c11 -Wall -Wextra -o hw-01.exe hw-01.c
```

### Run (each test is **one single line**)

**Linux/macOS:**

```bash
echo "(A (B () C))" | ./hw-01            # TRUE
echo "(A (B (C) D (E) F (G)))" | ./hw-01 # FALSE
echo "(A (B) (C))" | ./hw-01             # TRUE
echo "(A (B C) (D))" | ./hw-01           # TRUE
echo "((A)" | ./hw-01                    # ERROR
echo "" | ./hw-01                        # ERROR
```

**Windows (PowerShell):**

```powershell
echo "(A (B () C))" | .\hw-01.exe            # TRUE
echo "(A (B (C) D (E) F (G)))" | .\hw-01.exe # FALSE
echo "(A (B) (C))" | .\hw-01.exe             # TRUE
echo "(A (B C) (D))" | .\hw-01.exe           # TRUE
echo "((A)" | .\hw-01.exe                    # ERROR
echo "" | .\hw-01.exe                        # ERROR
```

### Notes

* The program reads **exactly one line** from stdin. Inputs containing internal newlines are considered **ERROR** (per assignment).
* Whitespace is ignored.
* A null node is written as `()`.
* By default, `()` as the **entire input** is treated as **TRUE** (an empty binary tree).
  If your course requires a different behavior, adjust the `res == 0` branch in `main`.

**Interactive input (optional):**

* Linux/macOS: run `./hw-01`, paste the line, press **Ctrl+D**.
* Windows PowerShell: run `.\hw-01.exe`, paste the line, press **Ctrl+Z** then **Enter**.
