/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/Nova-Physics

*/

#include "example_base.h"


int main(int argc, char *argv[]) {
    // Create example
    Example *example = Example_new(
        1280, 720,
        "Nova Physics — Arch Example",
        165.0,
        1.0/60.0
    );

    example->substeps = 5;

    nv_Body *ground = nv_Rect_new(
        nv_BodyType_STATIC,
        (nv_Vector2){64.0, 52.5},
        0.0,
        2.0,
        NV_COR_WOOD,
        185.0, 5.0
    );

    nv_Space_add(example->space, ground);


    nv_Vector2Array *vertices;
    nv_Vector2 center;
    nv_Body *brick;

    nv_BodyType type = nv_BodyType_DYNAMIC;

    vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(vertices, (nv_Vector2){3.0, 10.0});
    nv_Vector2Array_add(vertices, (nv_Vector2){3.0, -10.0});
    nv_Vector2Array_add(vertices, (nv_Vector2){-3.0, -10.0});
    nv_Vector2Array_add(vertices, (nv_Vector2){-3.0, 10.0});

    center = (nv_Vector2){37.0, 40.0};
    brick = nv_Polygon_new(type, center, 0.0, 7.0, NV_COR_WOOD, vertices);
    nv_Space_add(example->space, brick);

    vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(vertices, (nv_Vector2){2.6294875282488817, 2.0104255255025123});
    nv_Vector2Array_add(vertices, (nv_Vector2){3.312303655305328, -1.422322955908794});
    nv_Vector2Array_add(vertices, (nv_Vector2){-2.5712787118030915, -2.5985280950962193});
    nv_Vector2Array_add(vertices, (nv_Vector2){-3.3705124717511183, 2.0104255255025123});

    center = (nv_Vector2){37.37051247175112, 27.989574474497488};
    brick = nv_Polygon_new(type, center, 0.0, 2.0, NV_COR_WOOD, vertices);
    nv_Space_add(example->space, brick);
    
    vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(vertices, (nv_Vector2){2.183643255897812, 2.492948272150943});
    nv_Vector2Array_add(vertices, (nv_Vector2){3.523035269175625, -0.740630091638559});
    nv_Vector2Array_add(vertices, (nv_Vector2){-2.006739413862829, -3.0690613134759017});
    nv_Vector2Array_add(vertices, (nv_Vector2){-3.6999391112106084, 1.3167431329635177});

    center = (nv_Vector2){38.499172871158635, 24.07430324643775};
    brick = nv_Polygon_new(type, center, 0.0, 2.0, NV_COR_WOOD, vertices);
    nv_Space_add(example->space, brick);
    
    vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(vertices, (nv_Vector2){1.6450560399843028, 2.888719465320071});
    nv_Vector2Array_add(vertices, (nv_Vector2){3.5895518555529122, -0.021424177738836647});
    nv_Vector2Array_add(vertices, (nv_Vector2){-1.3498892524830695, -3.4275835310639677});
    nv_Vector2Array_add(vertices, (nv_Vector2){-3.884718643054151, 0.5602882434827279});

    center = (nv_Vector2){40.37715210034996, 20.44495368947912};
    brick = nv_Polygon_new(type, center, 0.0, 2.0, NV_COR_WOOD, vertices);
    nv_Space_add(example->space, brick);
    
    vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(vertices, (nv_Vector2){1.0288202961227284, 3.178114191291493});
    nv_Vector2Array_add(vertices, (nv_Vector2){3.503694030275642, 0.7032404571385769});
    nv_Vector2Array_add(vertices, (nv_Vector2){-0.6218935144851117, -3.653309486396438});
    nv_Vector2Array_add(vertices, (nv_Vector2){-3.9106208119132533, -0.2280451620336379});

    center = (nv_Vector2){42.93788365978014, 17.245415320448792};
    brick = nv_Polygon_new(type, center, 0.0, 2.0, NV_COR_WOOD, vertices);
    nv_Space_add(example->space, brick);
    
    vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(vertices, (nv_Vector2){0.3551151314970184, 3.343370585391091});
    nv_Vector2Array_add(vertices, (nv_Vector2){3.265258774555929, 1.3988747698224813});
    nv_Vector2Array_add(vertices, (nv_Vector2){0.15009850721081647, -3.729065997069655});
    nv_Vector2Array_add(vertices, (nv_Vector2){-3.770472413263735, -1.013179358143924});

    center = (nv_Vector2){46.086462558558765, 14.605285192196277};
    brick = nv_Polygon_new(type, center, 0.0, 2.0, NV_COR_WOOD, vertices);
    nv_Space_add(example->space, brick);
    
    vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(vertices, (nv_Vector2){-0.3506082644408764, 3.370289727426338});
    nv_Vector2Array_add(vertices, (nv_Vector2){2.8829700993486314, 2.0308977141485243});
    nv_Vector2Array_add(vertices, (nv_Vector2){0.9334066968782337, -3.643536402109065});
    nv_Vector2Array_add(vertices, (nv_Vector2){-3.4657685317859888, -1.7576510394657987});

    center = (nv_Vector2){49.70232959755557, 12.63387023459242};
    brick = nv_Polygon_new(type, center, 0.0, 2.0, NV_COR_WOOD, vertices);
    nv_Space_add(example->space, brick);
    
    vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(vertices, (nv_Vector2){-1.0581733016350996, 3.2502594968477267});
    nv_Vector2Array_add(vertices, (nv_Vector2){2.374575179776207, 2.5674433697912775});
    nv_Vector2Array_add(vertices, (nv_Vector2){1.691334825964384, -3.3935282472291464});
    nv_Vector2Array_add(vertices, (nv_Vector2){-3.007736704105497, -2.4241746194098623});

    center = (nv_Vector2){53.6434729985393, 11.414508451893218};
    brick = nv_Polygon_new(type, center, 0.0, 2.0, NV_COR_WOOD, vertices);
    nv_Space_add(example->space, brick);
    
    vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(vertices, (nv_Vector2){-1.7341826665139024, 2.9822138308846364});
    nv_Vector2Array_add(vertices, (nv_Vector2){1.7658173334860976, 2.9822138308846364});
    nv_Vector2Array_add(vertices, (nv_Vector2){2.3857883533535413, -2.9856698756334894});
    nv_Vector2Array_add(vertices, (nv_Vector2){-2.4174230203257254, -2.9787577861357875});

    center = (nv_Vector2){57.75223084482941, 10.99973799079986};
    brick = nv_Polygon_new(type, center, 0.0, 2.0, NV_COR_WOOD, vertices);
    nv_Space_add(example->space, brick);
    
    vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(vertices, (nv_Vector2){-2.34445808274927, 2.574004053782724});
    nv_Vector2Array_add(vertices, (nv_Vector2){1.0882903986620363, 3.2568201808391732});
    nv_Vector2Array_add(vertices, (nv_Vector2){2.98065474696906, -2.436944581886499});
    nv_Vector2Array_add(vertices, (nv_Vector2){-1.7244870628818263, -3.3938796527354014});

    center = (nv_Vector2){61.86250626106478, 11.40794776790177};
    brick = nv_Polygon_new(type, center, 0.0, 2.0, NV_COR_WOOD, vertices);
    nv_Space_add(example->space, brick);
    
    vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(vertices, (nv_Vector2){-2.8570719003619502, 2.04270484234236});
    nv_Vector2Array_add(vertices, (nv_Vector2){0.3765064634275518, 3.3820968556201736});
    nv_Vector2Array_add(vertices, (nv_Vector2){3.4452729889893363, -1.7737417775792266});
    nv_Vector2Array_add(vertices, (nv_Vector2){-0.9647075520549265, -3.6510599203833123});

    center = (nv_Vector2){65.80786856008876, 12.622063106398585};
    brick = nv_Polygon_new(type, center, 0.0, 2.0, NV_COR_WOOD, vertices);
    nv_Space_add(example->space, brick);
    
    vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(vertices, (nv_Vector2){-3.2453796908309527, 1.4136316034322163});
    nv_Vector2Array_add(vertices, (nv_Vector2){-0.3352360477720481, 3.358127419000823});
    nv_Vector2Array_add(vertices, (nv_Vector2){3.757228903872169, -1.0295519926658585});
    nv_Vector2Array_add(vertices, (nv_Vector2){-0.17661316526916834, -3.742207029767184});

    center = (nv_Vector2){69.42975471434727, 14.590528358586543};
    brick = nv_Polygon_new(type, center, 0.0, 2.0, NV_COR_WOOD, vertices);
    nv_Space_add(example->space, brick);
    
    vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(vertices, (nv_Vector2){-3.4904233912835823, 0.7182140723195743});
    nv_Vector2Array_add(vertices, (nv_Vector2){-1.0155496571306686, 3.193087806472491});
    nv_Vector2Array_add(vertices, (nv_Vector2){3.903931488053604, -0.24183653944495517});
    nv_Vector2Array_add(vertices, (nv_Vector2){0.6020415603606353, -3.669465339347107});

    center = (nv_Vector2){72.5849420578588, 17.23044170526779};
    brick = nv_Polygon_new(type, center, 0.0, 2.0, NV_COR_WOOD, vertices);
    nv_Space_add(example->space, brick);
    
    vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(vertices, (nv_Vector2){-3.582269061011425, -0.008817472098340318});
    nv_Vector2Array_add(vertices, (nv_Vector2){-1.6377732454428156, 2.901326170960567});
    nv_Vector2Array_add(vertices, (nv_Vector2){3.882830222281382, 0.5512331191535651});
    nv_Vector2Array_add(vertices, (nv_Vector2){1.3372120841728474, -3.4437418180157864});

    center = (nv_Vector2){75.15166146173956, 20.43234698383862};
    brick = nv_Polygon_new(type, center, 0.0, 2.0, NV_COR_WOOD, vertices);
    nv_Space_add(example->space, brick);
    
    vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(vertices, (nv_Vector2){-3.5201681652513686, -0.7323563380462105});
    nv_Vector2Array_add(vertices, (nv_Vector2){-2.180776151973555, 2.5012220257432944});
    nv_Vector2Array_add(vertices, (nv_Vector2){3.7005090147520834, 1.313583702156123});
    nv_Vector2Array_add(vertices, (nv_Vector2){2.000435302472829, -3.0824493898532124});

    center = (nv_Vector2){77.03405638154811, 24.0660294928454};
    brick = nv_Polygon_new(type, center, 0.0, 2.0, NV_COR_WOOD, vertices);
    nv_Space_add(example->space, brick);
    
    vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(vertices, (nv_Vector2){-3.3117293552096383, -1.4194646598088583});
    nv_Vector2Array_add(vertices, (nv_Vector2){-2.628913228153192, 2.0132838216024482});
    nv_Vector2Array_add(vertices, (nv_Vector2){3.371086771846808, 2.0132838216024482});
    nv_Vector2Array_add(vertices, (nv_Vector2){2.5695558115159995, -2.6071029833960297});

    center = (nv_Vector2){78.1650095847842, 27.98671617839755};
    brick = nv_Polygon_new(type, center, 0.0, 2.0, NV_COR_WOOD, vertices);
    nv_Space_add(example->space, brick);
    
    vertices = nv_Vector2Array_new();
    nv_Vector2Array_add(vertices, (nv_Vector2){-3.0, -10.0});
    nv_Vector2Array_add(vertices, (nv_Vector2){-3.0, 10.0});
    nv_Vector2Array_add(vertices, (nv_Vector2){3.0, 10.0});
    nv_Vector2Array_add(vertices, (nv_Vector2){3.0, -10.0});

    center = (nv_Vector2){78.536096356631, 40.0};
    brick = nv_Polygon_new(type, center, 0.0, 7.0, NV_COR_WOOD, vertices);
    nv_Space_add(example->space, brick);
    

    for (size_t i = 0; i < example->space->bodies->size; i++) {
        nv_Body *body = example->space->bodies->data[i];

        body->static_friction = 3.0;
        body->dynamic_friction = 0.7;
    }


    Example_run(example);

    Example_free(example);

    return 0;
}