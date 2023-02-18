# Body
A rigid body is a non deformable object with mass in space. Bodies can be dynamic or static in the simulation.



## Body types

#### Dynamic (`nv_BodyType_DYNAMIC`)
Dynamic bodies are affected by all forces and other bodies in the simulation.

#### Static (`nv_BodyType_STATIC`)
Static bodies have infinite mass and inertia and doesn't get affected by any forces.



## Body shapes
Currently Nova Physics doesn't store multiple shapes per body, **_yet_**. Instead, each body represents one geometry.

### Circle (`nv_BodyShape_CIRCLE`)
Circle bodies only stores radius. You should use `nv_Circle_new()` function to create bodies with circle shape.

### Polygon (`nv_BodyShape_POLYGON`)
Polygon bodies are defined by its vertices, which are `nv_Array` of `nv_Vector2` objects. You should use `nv_Polygon_new()` function to create bodies with polygon shape.

### Rectangle
There isn't any specific shape type for rectangles or boxes. You can use `nv_Rect_new()` function to create a polygon body with a rectangle dimensions instead of calculating vertices manually.



# Fields
### `nv_BodyType type`
Type of the body.

### `nv_BodyShape shape`
Shape of the body.

### `nv_Vector2 position`
Position of the body in meters. This is also the center of mass, if you created a polygon with a offset center of mass, the simulation accuracy may suffer. Use `nv_polygon_centroid()` to get your vertices' center of mass.

### `double angle`
Angle of the body in radians.

### `nv_Vector2 linear_velocity`
Linear velocity of the body.

### `double angular_velocity`
Angular velocity of the body.

### `double linear_damping`
Linear velocity damping of the body. Defines how much of the linear velocity is reduced over time.

### `double angular_damping`
Angular velocity damping of the body. Defines how much of the angular velocity is reduced over time.

### `nv_Vector2 force`
Force applied on the body.

### `double torque`
Torque applied on the body.

### `nv_Material material`
Material of the body.

### `double mass`
Mass of the body.

### `double invmass`
1 / mass, used in internal calculations.

### `double inertia`
Moment of inertia of the body.

### `double invinertia`
1 / inertia, used in internal calculations.

### `bool is_sleeping`
Flag indicating whether the body is asleep or not.

### `int sleep_counter`
Internal counter to calculate body's awake time.

### `bool is_attractor`
Flag indicating whether the body is an attractor or not. Use `nv_Body_set_is_attractor()` and `nv_Body_get_is_attractor()` methods.

### `double radius`
Radius of the circle body. This is only used if the body shape is circle.

### `nv_Array *vertices`
Vertices of the polygon body. This is only used if the body shape is polygon.



# Creating & Destroying
`nv_Body_new` allocates and initializes new body instances, but for convenience there are factory functions that generates the necessary body definitons for you.

### Creating a circle body
`nv_Circle_new()` function is for creating bodies with circle shape.
```c
nv_Body *circle_body = nv_Circle_new(
    nv_BodyType_STATIC,  // Body type
    nv_Vector2_zero,     // Position
    0.0,                 // Angle
    nv_Material_GLASS,   // Material
    5.0                  // Radius
);
```

### Creating a polygon body
`nv_Polygon_new()` function is for creating bodies with polygon shape.
```c
nv_Array *vertices = nv_Array_new();
nv_Array_add(vertices, nv_Vector2_heap(-5.0, -3.0));
nv_Array_add(vertices, nv_Vector2_heap( 5.0, -3.0));
nv_Array_add(vertices, nv_Vector2_heap( 3.0,  3.0));
nv_Array_add(vertices, nv_Vector2_heap(-3.0,  3.0));

nv_Body *poly_body = nv_Polygon_new(
    nv_BodyType_DYNAMIC,     // Body type
    (nv_Vector2){10.0, 0.0}, // Position
    NV_PI / 2.0,             // Angle
    nv_Material_STEEL,       // Material
    vertices                 // Vertices
);
```

### Creating a rectangle / box body
`nv_Rect_new()` function is for creating rectangle bodies with polygon shape.
```c
nv_Material slippery = {2.0, 0.05, 0.0, 0.0};

nv_Body *box = nv_Rect_new(
    nv_BodyType_DYNAMIC,    // Body type
    (nv_Vector2){5.0, 5.0}, // Position
    0.0,                    // Angle
    slippery,               // Material
    9.0,                    // Width
    7.5                     // Height
);
```

### Destroying bodies
You **should not be** freeing bodies manually. When the simulation ends and `nv_Space_free()` is called, all bodies that are attached to space will be freed automatically. But in case you have to do it manually: `nv_Body_free(void *body)` destroys body and all the other memory it allocated.



# Adding Bodies to Space
You should add `nv_Space_add(nv_Space *space, nv_Body *body)` method to add bodies to space. Do not add or remove bodies directly from space's `bodies` array.

# Attractor Bodies & Attractive Forces
Attractor bodies apply forces to other bodies around it towards itself. You can play around with the `NV_GRAV_CONST` or  `NV_GRAV_SCALE` to adjust attractive forces.

### `void nv_Body_set_is_attractor(nv_Body *body, bool is_attractor)`
Use this method to set a body as attractor.

### `void nv_Body_get_is_attractor(nv_Body *body)`
Use this method to get body's attractor flag.



# Applying Forces
It's best to apply forces on dynamic bodies to move them instead of directly changing velocity or position attributes.

### `void nv_Body_apply_force(nv_Body *body, nv_Vector2 force)`
Applies force at the center of mass of the body.

### `void nv_Body_apply_force_at(nv_Body *body, nv_Vector2 force, nv_Vector2 position)`
Applies force and torque at the given local point of the body.



# Energy
You can use body's energy functions to get linear and rotational kinetic energy.

### `double nv_Body_get_kinetic_energy(nv_Body *body)`
Returns body's linear kinetic energy.

### `double nv_Body_get_rotational_energy(nv_Body *body)`
Returns body's rotational kinetic energy.
