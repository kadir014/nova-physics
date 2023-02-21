# Collisions
Collision module of Nova Physics has various functions to detect intersections between different shapes. Some functions also calculates penetration depth & collision normal and returns `nv_Resolution`.
- [Circle x Circle](#circle-x-circle)
- [Circle x Point](#circle-x-point)
- [Polygon x Circle](#circle-x-circle)
- [Polygon x Polygon](#circle-x-circle)
- [Polygon x Point](#polygon-x-point)
- [AABB x AABB](#circle-x-circle)
- [AABB x Point](#circle-x-circle)


## Circle x Circle
### `nv_Resolution nv_collide_circle_x_circle(nv_Body *a, nv_Body *b)`
Calculate the collision between two circles

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/docs/assets/collision_circlexcircle.png" width=620>


## Circle x Point
### `bool nv_collide_circle_x_point(nv_Body *circle, nv_Vector2 point)`
Check if point is inside circle.

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/docs/assets/collision_circlexpoint.png" width=620>


## Polygon x Circle
### `nv_Resolution nv_collide_polygon_x_circle(nv_Body *polygon, nv_Body *circle)`
Calculate the collision between polygon and circle

<img src="https://raw.githubusercontent.com/kadir014/nova-physics/main/docs/assets/collision_polygonxcircle.png" width=620>


## Polygon x Polygon
### `nv_Resolution nv_collide_polygon_x_polygon(nv_Body *a, nv_Body *b)`
Calculate the collision bettween polygons


## Polygon x Point
### `bool nv_collide_polygon_x_point(nv_Body *polygon, nv_Vector2 point)`
Check if point is inside polygon


## AABB x AABB
### `bool nv_collide_aabb_x_aabb(nv_AABB a, nv_AABB b)`
Check if two AABBs collide


## AABB x Point
### `bool nv_collide_aabb_x_point(nv_AABB aabb, nv_Vector2 point)`
Check if point is inside AABB