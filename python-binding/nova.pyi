from typing import overload, Tuple


STATIC: int
DYNAMIC: int
CIRCLE: int
POLYGON: int


class Vector2:
    x: float
    y: float

    @overload
    def __init__(self, x: float, y: float) -> None: ...

    @overload
    def __add__(self, vector: Vector2) -> Vector2: ...

    @overload
    def __sub__(self, vector: Vector2) -> Vector2: ...

    @overload
    def __mul__(self, scalar: float) -> Vector2: ...

    @overload
    def __truediv__(self, scalar: float) -> Vector2: ...


class Space:
    @overload
    def step(self, dt: float, velocity_iters: int, position_iters: int, constraint_iters: int, substeps: int) -> None: ...

    @overload
    def add(self, body: "Body") -> None: ...

    @overload
    def get_bodies(self) -> Tuple["Body"]: ...


class Body:
    type: int
    shape: int
    position: Vector2
    angle: float
    radius: float

    @overload
    def __init__(self, type: int, shape: int, x: float, y: float, angle: float, radius: float) -> None: ...

    @overload
    def get_vertices(self) -> Tuple[float]: ...

    @overload
    def apply_force(self, x: float, y: float) -> None: ...


@overload
def create_circle(type: int, x: float, y: float, angle: float, radius: float) -> Body: ...

@overload
def create_rect(type: int, x: float, y: float, angle: float, width: float, height: float) -> Body: ...