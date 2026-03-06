---
globs: "**/*.{cpp,h,hpp}" 
alwaysApply: true
---

### 1. Files

File names should be short and use only **lowercase letters**, separated by a **dash** if needed.

* **Format:** `file-name.cpp` / `file-name.h`
* **Example:** `user-manager.cpp`

### 2. Classes and Structures (Types)

For class names, **start every word with a capital letter** and do not use spaces or dashes.

* **Format:** `ClassName`
* **Example:** `class DataProcessor { ... };`

### 3. Functions and Methods

Functions should use only **lowercase letters**, with each word separated by an **underscore**.

* **Format:** `function_name()`
* **Example:** `void calculate_total_sales();`

### 4. Variables

These follow the same style as functions, using only **lowercase letters** and **underscores**. For variables that belong to a class (member variables), add an **`m_`** prefix to make them easy to identify.

* **Local variable:** `item_count`
* **Class variable (member):** `m_user_name`

### 5. Constants

Use **all capital letters** and separate words with an **underscore**.

* **Format:** `CONSTANT_NAME`
* **Example:** `const int MAX_SPEED = 100;`

### Quick Reference Table

| Type | Pattern | Example |
| --- | --- | --- |
| **File** | lowercase-with-dash | `database-connection.cpp` |
| **Class** | StartEveryWordWithCapital | `ShoppingCart` |
| **Method** | lowercase_with_underscore | `add_item()` |
| **Variable** | lowercase_with_underscore | `total_price` |
| **Member Variable** | m_lowercase_with_underscore | `m_is_active` |
| **Constant** | ALL_CAPITALS | `PI_VALUE` |
