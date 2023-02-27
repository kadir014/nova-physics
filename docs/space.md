# Space
Space is the host of all objects and the simulation itself. Space's gravity is parallel all across the space unlike [bodies' attractive forces](TODO)

### `nv_Array *bodies`
Array of bodies the space is responsible for. Don't use `nv_Array_add`, instead use `nv_Space_add`

### `nv_Array *attractors`
Array of bodies that are attractors. Do not interact with this array yourself, space and bodies handle it.

### `nv_Array *constraints`
Array of constraints the space is responsible for. Don't use `nv_Array_add`, instead use `nv_Space_add_constraint`

### `nv_Array *res`
Array of active collision resolution objects.

### `nv_Vector2 gravity`
Gravity vector.

### `bool sleeping`
Enable/disable sleeping.

### `bool warmstarting`
Enable/disable warm starting from last step's accumulated impulses.

### `nv_float baumgarte`
Constant value used in calculation of Baumgarte stabilization. In range 0.0, 1.0. Usually close to zero, 0.1 or 0.2.

### `nv_CoefficientMix mix_restitution`
Mixing function used to mix two material's restitution coefficients.

### `nv_CoefficientMix mix_friction`
Mixing function used to mix two material's friction coefficients.

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
### `void nv_Space_step(nv_Space *space, double dt, int velocity_iters, int position_iters, int substeps)`
This is the main function that advances the simulation.
- `double dt`: Time step (delta time).
- `int velocity_iters`: Velocity solving iterations. Around 8-12 is good enough, you have to play around to fine-tune it.
- `int position_iters`: Position (pseudo-velocity) iterations. Around 3-5 is good enough, you have to play around to fine-tune it.
- `int substeps`: Sub steps count. This can affect performance heavily. In a scene with many objects only 1 sub step can cause sinking effect, for extra stability you may consider 2 or 3 substeps but always play around to see what's good enough for your case.
