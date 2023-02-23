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


/**
 * Nova physics engine module interface
 */
static PyModuleDef novamodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "nova",
    .m_doc = "Nova Physics Engine",
    .m_size = -1
};

/**
 * Nova physics engine module initializer
 */
PyMODINIT_FUNC PyInit_nova() {
    PyObject *m;

    if (PyType_Ready(&nv_SpaceType) < 0)
        return NULL;

    if (PyType_Ready(&nv_BodyObjectType) < 0)
        return NULL;


    m = PyModule_Create(&novamodule);
    if (m == NULL)
        return NULL;

    /**
     * Create nova.Space
     */
    Py_INCREF(&nv_SpaceType);
    if (PyModule_AddObject(m, "Space", (PyObject *) &nv_SpaceType) < 0) {
        Py_DECREF(&nv_SpaceType);
        Py_DECREF(m);
        return NULL;
    }

    /**
     * Create nova.Body
     */
    Py_INCREF(&nv_BodyObjectType);
    if (PyModule_AddObject(m, "Body", (PyObject *) &nv_BodyObjectType) < 0) {
        Py_DECREF(&nv_BodyObjectType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}