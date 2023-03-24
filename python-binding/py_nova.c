/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/Nova-Physics

*/

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "novaphysics/novaphysics.h"
#include "py_space.h"
#include "py_body.h"
#include "py_vector2.h"


/**
 * Nova Physics method interface
*/
static PyMethodDef nova_methods[] = {
    {
        "create_circle",
        nv_create_circle, METH_VARARGS,
        "Create a body with circle shape."
    },

    {
        "create_rect",
        nv_create_rect, METH_VARARGS,
        "Create a body with rect (polygon) shape."
    },

    {NULL, NULL, 0, NULL} /* Sentinel */
};

/**
 * Nova Physics module interface
 */
static PyModuleDef nova_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "nova",
    .m_doc = "Nova Physics Engine",
    .m_size = -1,
    .m_methods = nova_methods
};

/**
 * Nova Physics module initializer
 */
PyMODINIT_FUNC PyInit_nova() {
    PyObject *m;

    if (PyType_Ready(&nv_SpaceType) < 0)
        return NULL;

    if (PyType_Ready(&nv_BodyObjectType) < 0)
        return NULL;

    if (PyType_Ready(&nv_Vector2ObjectType) < 0)
        return NULL;


    m = PyModule_Create(&nova_module);
    if (m == NULL)
        return NULL;

    /**
     * Add nova.Space
     */
    Py_INCREF(&nv_SpaceType);
    if (PyModule_AddObject(m, "Space", (PyObject *) &nv_SpaceType) < 0) {
        Py_DECREF(&nv_SpaceType);
        Py_DECREF(m);
        return NULL;
    }

    /**
     * Add nova.Body
     */
    Py_INCREF(&nv_BodyObjectType);
    if (PyModule_AddObject(m, "Body", (PyObject *) &nv_BodyObjectType) < 0) {
        Py_DECREF(&nv_BodyObjectType);
        Py_DECREF(m);
        return NULL;
    }

    /**
     * Add nova.Vector2
     */
    Py_INCREF(&nv_Vector2ObjectType);
    if (PyModule_AddObject(m, "Vector2", (PyObject *) &nv_Vector2ObjectType) < 0) {
        Py_DECREF(&nv_Vector2ObjectType);
        Py_DECREF(m);
        return NULL;
    }

    /* Add constants */

    PyModule_AddIntConstant(m, "STATIC",  nv_BodyType_STATIC);
    PyModule_AddIntConstant(m, "DYNAMIC", nv_BodyType_DYNAMIC);
    PyModule_AddIntConstant(m, "CIRCLE",  nv_BodyShape_CIRCLE);
    PyModule_AddIntConstant(m, "POLYGON", nv_BodyShape_POLYGON);

    return m;
}