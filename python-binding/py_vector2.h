/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/Nova-Physics

*/

#ifndef NV_PYTHON_VECTOR2_H
#define NV_PYTHON_VECTOR2_H

#include <Python.h>
#include "structmember.h"
#include "novaphysics/novaphysics.h"


/**
 * Vector2 object interface
 */
typedef struct {
    PyObject_HEAD
    double x;
    double y;
} nv_Vector2Object;

static nv_Vector2Object *nv_Vector2Object_new(double x, double y);

#define PY_TO_VEC2(o) (NV_VEC2((o)->x, (o)->y))


/**
 * Vector2 object deallocater
 */
static void nv_Vector2Object_dealloc(nv_Vector2Object *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

/**
 * Vector2 object initializer
 */
static int nv_Vector2Object_init(
    nv_Vector2Object *self,
    PyObject *args,
    PyObject *kwds
) {
    double x;
    double y;

    if (!PyArg_ParseTuple(args, "dd", &x, &y))
        return NULL;

    self->x = x;
    self->y = y;

    return 0;
}

static PyObject *nv_Vector2Object___add__(
    nv_Vector2Object *self,
    nv_Vector2Object *vector
);

static PyObject *nv_Vector2Object___sub__(
    nv_Vector2Object *self,
    nv_Vector2Object *vector
);

static PyObject *nv_Vector2Object___mul__(
    nv_Vector2Object *self,
    PyObject *scalar
);

static PyObject *nv_Vector2Object___truediv__(
    nv_Vector2Object *self,
    PyObject *scalar
);

/**
 * Vector2 object method interface
 */
static PyMethodDef nv_Vector2Object_methods[] = {
    {
        "__add__",
        (PyCFunction)nv_Vector2Object___add__, METH_VARARGS,
        ""
    },

    {NULL} // Sentinel
};

static PyNumberMethods nv_Vector2Object_operators = {
    .nb_add =         nv_Vector2Object___add__,
    .nb_subtract =    nv_Vector2Object___sub__,
    .nb_multiply =    nv_Vector2Object___mul__,
    .nb_true_divide = nv_Vector2Object___truediv__
};

/**
 * Vector2 object member interface
 */
static PyMemberDef nv_Vector2Object_members[] = {
    {
        "x",
        T_DOUBLE, offsetof(nv_Vector2Object, x), 0,
        "X component of the vector"
    },

    {
        "y",
        T_DOUBLE, offsetof(nv_Vector2Object, y), 0,
        "Y component of the vector"
    },

    {NULL} // Sentinel
};

/**
 * Vector2 type internals
 */
static PyTypeObject nv_Vector2ObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "nova.Vector2",
    .tp_doc = "Vector2 object",
    .tp_basicsize = sizeof(nv_Vector2Object),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_dealloc = (destructor)nv_Vector2Object_dealloc,
    .tp_init = (initproc)nv_Vector2Object_init,
    .tp_members = nv_Vector2Object_members,
    .tp_as_number = &nv_Vector2Object_operators
};


static nv_Vector2Object *nv_Vector2Object_new(double x, double y) {
    PyObject *args = Py_BuildValue("dd", x, y);
    nv_Vector2Object *obj = (nv_Vector2Object *)PyObject_CallObject((PyObject *)&nv_Vector2ObjectType, args);
    Py_DECREF(args);
    Py_INCREF(obj);
    return obj;
}

static PyObject *nv_Vector2Object___add__(
    nv_Vector2Object *self,
    nv_Vector2Object *vector
) {
    nv_Vector2 result = nv_Vector2_add(PY_TO_VEC2(self), PY_TO_VEC2(vector));

    return nv_Vector2Object_new(result.x, result.y);
}

static PyObject *nv_Vector2Object___sub__(
    nv_Vector2Object *self,
    nv_Vector2Object *vector
) {
    nv_Vector2 result = nv_Vector2_sub(PY_TO_VEC2(self), PY_TO_VEC2(vector));

    return nv_Vector2Object_new(result.x, result.y);
}

static PyObject *nv_Vector2Object___mul__(
    nv_Vector2Object *self,
    PyObject *scalar
) {
    double s = PyFloat_AS_DOUBLE(scalar);

    nv_Vector2 result = nv_Vector2_muls(PY_TO_VEC2(self), s);

    return nv_Vector2Object_new(result.x, result.y);
}

static PyObject *nv_Vector2Object___truediv__(
    nv_Vector2Object *self,
    PyObject *scalar
) {
    double s = PyFloat_AS_DOUBLE(scalar);

    nv_Vector2 result = nv_Vector2_divs(PY_TO_VEC2(self), s);

    return nv_Vector2Object_new(result.x, result.y);
}


#endif