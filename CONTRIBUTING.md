# Contributing to Nova Physics Engine

First off, thanks for taking the time to contribute!

All types of contributions are encouraged and valued. See the categories below for different ways to help and details about how this project handles them. Please make sure to read the relevant section before making your contribution.

And if you like the project, but just don't have time to contribute, that's fine. You can leave a ⭐ to show your appreciation :)



## ❔ I Have a Question

Before you ask a question, it is best to search for existing [Issues](/issues) that might help you. In case you have found a suitable issue and still need clarification, you can write your question in this issue.

If you then still feel the need to ask a question and need clarification, we recommend the following:

- Open an [Issue](/issues/new).
- Provide as much context as you can about what you're running into.
- Provide project and platform versions depending on what seems relevant.

We will then take care of the issue as soon as possible.



## 🐛 Reporting Bugs

A good bug report shouldn't leave others needing to chase you up for more information. Therefore, we ask you to investigate carefully, collect information and describe the issue in detail in your report.

- Make sure that you are using the **latest** version.
- Determine if your bug is really a bug and not an error on your side.
- Check if other users have experienced (and potentially already solved) the same issue you are having.
- Once you are sure you are having an issue, open an [issue](/issues/new). 
- Explain the behavior you would expect and the actual behavior.
- Please provide **as much context as possible** and describe the _reproduction steps_ that someone else can follow to recreate the issue on their own. 



## ✨ Suggesting Enhancements & Features

This section is for submitting enhancement suggestions, including completely new features, minor improvements to existing functionality and so on. Please while suggesting an enhancement or feature:

- Make sure that you are using the latest version.
- Check out the documentation to find out if the functionality already exists.
- Check out [Issues](/issues) to find out if it has been already suggested. If so, add a comment instead opening a new issue.
- Find out whether your idea fits with the scope and aims of the project. It's up to you to make a strong case to convince developers of the merits of this feature. 
- Once you are sure that the new functionality fits the project, create a new [issue](/issues/new) with a **descriptive title**.
- Provide a **clear, step-by-step description** of the suggested enhancement in as many details as possible.
- You may also want to include screenshots or GIFs to help demonstrate the functionality.
- **Explain why this enhancement would be useful** to Nova Physics Engine and the majority of the users.



## 🖋️ Style Guide

It is best to follow the code style guide while contributing to keep a consistent codebase.

`nv` prefix is used accross the library.

### Structs & Methods & Functions
```c
// Struct names are in PascalCase, methods are in snake_case
typedef struct {
    ...
} nvSomeStruct;

// Initializer / constructor method
nvSomeStruct *nvSomeStruct_new();

// Destroyer / deconstructor method
void nvSomeStruct_free(nv_SomeStruct *some_struct);

// Random method
type nvSomeStruct_random_method(...);

// Library functions are in snake_case
void nv_some_function(...);
```

### Macros & Constants & Enums
```c
// Macros and constants are in SCREAMING_SNAKE_CASE (including the prefix)
#define NV_SOME_CONSTANT 31;
#define NV_SOME_MACRO(x) (x * x + 1)

// Enums are in PascalCase, with their fields being the enum name + field name in full caps
typedef enum {
    nvSomeEnum_FIELD1,
    nvSomeEnum_FIELD2,
    ...
} nvSomeEnum;
```