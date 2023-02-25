#define NV_USE_FLOAT
#include "novaphysics/novaphysics.h"


int main() {
    nv_Body *body = nv_Body_new(
        nv_BodyType_STATIC,
        nv_BodyShape_POLYGON,
        NV_VEC2(0.0, 0.1),
        12.62,
        nv_Material_WOOD,
        0.0f,
        NULL
    );

    nv_Body_free(body);
}