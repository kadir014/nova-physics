/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/Nova-Physics

*/

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "novaphysics/novaphysics.h"


/**
 * python-binding.c
 * 
 * Python binding for Nova Physics Engine
 */


// /**
//  * Body object interface
//  */
// typedef struct {
//     PyObject_HEAD
//     nv_Body *body;
// } nv_BodyObject;

// /**
//  * Body object deallocater
//  */
// static void nv_BodyObject_dealloc(nv_BodyObject *self) {
//     nv_Body_free(self->body);
//     Py_TYPE(self)->tp_free((PyObject *) self);
// }

// /**
//  * Body type internals
//  */
// static PyTypeObject nv_BodyObjectType = {
//     PyVarObject_HEAD_INIT(NULL, 0)
//     .tp_name = "nova.Body",
//     .tp_doc = "Body object",
//     .tp_basicsize = sizeof(nv_BodyObject),
//     .tp_itemsize = 0,
//     .tp_flags = Py_TPFLAGS_DEFAULT,
//     .tp_new = PyType_GenericNew,
//     .tp_dealloc = (destructor)nv_BodyObject_dealloc
// };


/**
 * Space object interface
 */
typedef struct {
    PyObject_HEAD
    nv_Space *space;
} nv_SpaceObject;

/**
 * Space object deallocater
 */
static void nv_SpaceObject_dealloc(nv_SpaceObject *self) {
    nv_Space_free(self->space);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

/**
 * Space object initializer
 */
static int nv_SpaceObject_init(
    nv_SpaceObject *self,
    PyObject *args,
    PyObject *kwds
) {
    // burada yeni space oluşturma hata kontrol et, hata ise -1 döndür
    self->space = nv_Space_new();

    nv_Body *circle1 = nv_Circle_new(
        nv_BodyType_DYNAMIC, nv_Vector2_zero, 0.0, nv_Material_WOOD, 3.0);

    nv_Space_add(self->space, circle1);

    nv_Body *circle2 = nv_Circle_new(
        nv_BodyType_DYNAMIC, (nv_Vector2){0.4, 1.2}, 0.0, nv_Material_WOOD, 3.0);

    nv_Space_add(self->space, circle2);

    return 0;
}

/**
 * Space.get_bodies() method
 * Returns nv_Space instance's bodies array as a Python tuple
 */
static PyObject *nv_SpaceObject_get_bodies(
    nv_SpaceObject *self,
    PyObject *Py_UNUSED(ignored)
) {
    nv_Array *bodies = self->space->bodies;

    PyObject *return_tup = PyTuple_New(bodies->size);

    PyObject *body_tup;
    PyObject *body_x;
    PyObject *body_y;
    PyObject *body_a;
    PyObject *body_r;

    for (size_t i = 0; i < bodies->size; i++) {
        nv_Body *body = (nv_Body *)bodies->data[i];

        body_tup = PyTuple_New(4);
        body_x = PyFloat_FromDouble(body->position.x);
        body_y = PyFloat_FromDouble(body->position.y);
        body_a = PyFloat_FromDouble(body->angle);
        body_r = PyFloat_FromDouble(body->radius);
        PyTuple_SET_ITEM(body_tup, 0, body_x);
        PyTuple_SET_ITEM(body_tup, 1, body_y);
        PyTuple_SET_ITEM(body_tup, 2, body_a);
        PyTuple_SET_ITEM(body_tup, 3, body_r);
        
        PyTuple_SET_ITEM(return_tup, i, body_tup);
    }

    return return_tup;
}

/**
 * Space.step() method
 */
static PyObject *nv_SpaceObject_step(
    nv_SpaceObject *self,
    PyObject *Py_UNUSED(ignored)
) {
    nv_Space_step(self->space, 1.0/60.0, 8, 1);
    Py_RETURN_NONE;
}

/**
 * Space object method interface
 */
static PyMethodDef nv_SpaceObject_methods[] = {
    {"get_bodies", (PyCFunction) nv_SpaceObject_get_bodies, METH_NOARGS,
     "Get bodies"
    },
    {"step", (PyCFunction) nv_SpaceObject_step, METH_NOARGS,
     "Advance the simulation"
    },
    {NULL}  /* Sentinel */
};

/**
 * Space type internals
 */
static PyTypeObject nv_SpaceType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "nova.Space",
    .tp_doc = "Space object",
    .tp_basicsize = sizeof(nv_SpaceObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_dealloc = (destructor)nv_SpaceObject_dealloc,
    .tp_init = (initproc)nv_SpaceObject_init,
    .tp_methods = nv_SpaceObject_methods
};


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

    return m;
}