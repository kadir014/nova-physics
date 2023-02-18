# Space
Space is the host of all objects and the simulation itself. Space's gravity is parallel all across the space unlike [bodies' attractive forces](TODO)

```c
struct _nv_Space{
    nv_Array *bodies;
    nv_Array *attractors;

    nv_Vector2 gravity;
    
    bool sleeping;

    void *callback_user_data;
    nv_Space_callback before_collision;
    nv_Space_callback after_collision;
};
```

### `nv_Array *bodies`
Array of bodies the space is responsible for. Don't use `nv_Array_add`, instead use `nv_Space_add`

### `nv_Array *attractors`
Array of bodies that are attractors. Do not interact with this array yourself, space and bodies handle it.

### `nv_Vector2 gravity`
Gravity vector.

### `void *callback_user_data`
This is passed as an argument when space callbacks are called.

### `nv_Space_callback before_collision`
This callback is called before collisions are solved.

### `nv_Space_callback after_collision`
This callback is called after collisions are solved.



# Creating & Destroying
### `nv_Space *nv_Space_new()`
Create a new instance of `nv_Space` and return a pointer to it.

### `void nv_Space_free(nv_Space *space)`
Destroys space and all the other memory it allocated.

#### Example:
```c
nv_Space *space = nv_Space_new();

// Set the gravity to Moon's. Default is Eartch
space->gravity.y = NV_GRAVITY_MOON;

// Do stuff
...

nv_Space_free(space);
```

# Adding bodies
### `void nv_Space_add(nv_Body *body)`
Adds body to space.

#### Example:
```c
nv_Body *ball = nv_Circle_new(
    nv_BodyType_DYNAMIC,
    nv_Vector2_zero,
    0.0,
    nv_Material_STEEL,
    3.0
);

nv_Space_add(space, ball);
```

# Simulating the space
### `void nv_Space_step(nv_Space *space, double dt, int iterations, int substeps)`
This is the main functions that simulates the step.
- `double dt`: Time (in seconds) this step takes
- `int iterations`: Impulse resolving iterations. Around 4-8 is good enough, 1 may give unaccurate results.
- `int substeps`: How many sub steps. This can affect performance heavily. With many objects 1 sub step can result sinking.
