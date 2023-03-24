/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/Nova-Physics

*/

#ifndef NV_PYTHON_BODY_H
#define NV_PYTHON_BODY_H

#include <Python.h>
#include "structmember.h"
#include "novaphysics/novaphysics.h"
#include "py_vector2.h"


/**
 * Body object interface
 */
typedef struct {
    PyObject_HEAD
    nv_Body *body;
    nv_BodyType type;
    nv_BodyShape shape;
    nv_Vector2Object *position;
    double angle;
    double radius;
} nv_BodyObject;

/**
 * Body object deallocater
 */
static void nv_BodyObject_dealloc(nv_BodyObject *self) {
    // Don't free nv_Body instance because space frees it
    Py_XDECREF(self->position);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

/**
 * Body object initializer
 */
static int nv_BodyObject_init(
    nv_BodyObject *self,
    PyObject *args,
    PyObject *kwds
) {
    nv_BodyType type;
    nv_BodyShape shape;
    double x;
    double y;
    double angle;
    double radius;
    PyObject *vertices = NULL;

    if (!PyArg_ParseTuple(
        args, "iidddd|O",
        &type, &shape, &x, &y, &angle, &radius, &vertices
    ))
        return NULL;

    self->type = type;
    self->shape = shape;
    self->position = (nv_Vector2Object *)nv_Vector2Object_new(x, y);
    self->angle = angle;
    self->radius = radius;

    nv_Array *new_vertices = NULL;

    // Validate polygon vertices
    if (vertices) {
        if (!PySequence_Check(vertices)) {
            PyErr_SetString(PyExc_TypeError, "Vertices must be a sequence of number pairs");
            return 0;
        }

        size_t vertices_len = PySequence_Length(vertices);

        if (vertices_len < 3) {
            PyErr_SetString(PyExc_ValueError, "Polygon vertices must be at least length of 3");
            return 0;
        }

        // Create nv_Array from vertices sequence
        new_vertices = nv_Array_new();
        PyObject *v;
        PyObject *vx;
        PyObject *vy;

        for (size_t i = 0; i < vertices_len; i++) {
            v = PySequence_GetItem(vertices, i);
            vx = PySequence_GetItem(v, 0);
            vy = PySequence_GetItem(v, 1);

            nv_Array_add(new_vertices, NV_VEC2_NEW(PyFloat_AS_DOUBLE(vx), PyFloat_AS_DOUBLE(vy)));

            Py_DECREF(v);
            Py_DECREF(vx);
            Py_DECREF(vy);
        }
    }

    self->body = nv_Body_new(
        type,
        shape,
        NV_VEC2(x, y),
        angle,
        nv_Material_BASIC,
        radius,
        new_vertices
    );

    return 0;
}

/**
 * Body object member interface
 */
static PyMemberDef nv_BodyObject_members[] = {
    {
        "type",
        T_INT, offsetof(nv_BodyObject, type), 0,
        "Type of the body"
    },

    {
        "shape",
        T_INT, offsetof(nv_BodyObject, shape), 0,
        "Shape of the body"
    },

    {
        "position",
        T_OBJECT_EX, offsetof(nv_BodyObject, position), 0,
        "Position of the body"
    },

    {
        "angle",
        T_DOUBLE, offsetof(nv_BodyObject, angle), 0,
        "Angle"
    },

    {
        "radius",
        T_DOUBLE, offsetof(nv_BodyObject, radius), 0,
        "Radius"
    },

    {NULL} // Sentinel
};

/**
 * Body.get_vertices() method
 * Returns transformed vertices of the body
*/
static PyObject *nv_BodyObject_get_vertices(
    nv_BodyObject *self,
    PyObject *Py_UNUSED(ignored)
) {
    nv_Polygon_model_to_world(self->body);
    PyObject *return_tup = PyTuple_New(self->body->trans_vertices->size);

    PyObject *vertex_tup;

    for (size_t i = 0; i < self->body->trans_vertices->size; i++) {
        nv_Vector2 v = NV_TO_VEC2(self->body->trans_vertices->data[i]);

        PyTuple_SET_ITEM(return_tup, i, nv_Vector2Object_new(v.x, v.y));
    }

    return return_tup;
}

/**
 * Body.apply_force() method
 * Applies force to center of body
*/
static PyObject *nv_BodyObject_apply_force(
    nv_BodyObject *self,
    PyObject *args
) {
    nv_Vector2Object *force;

    if (!PyArg_ParseTuple(args, "O!", &nv_Vector2ObjectType, &force))
        return NULL;

    nv_Body_apply_force(self->body, NV_VEC2(force->x, force->y));

    Py_RETURN_NONE;
}

/**
 * Body object method interface
 */
static PyMethodDef nv_BodyObject_methods[] = {
    {
        "get_vertices",
        (PyCFunction)nv_BodyObject_get_vertices, METH_NOARGS,
        "Get the transformed vertices"
    },

    {
        "apply_force",
        (PyCFunction)nv_BodyObject_apply_force, METH_VARARGS,
        "Apply force at body center of mass"
    },

    {NULL} // Sentinel
};

/**
 * Body type internals
 */
static PyTypeObject nv_BodyObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "nova.Body",
    .tp_doc = "Body object",
    .tp_basicsize = sizeof(nv_BodyObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_dealloc = (destructor)nv_BodyObject_dealloc,
    .tp_init = (initproc)nv_BodyObject_init,
    .tp_methods = nv_BodyObject_methods,
    .tp_members = nv_BodyObject_members
};


static PyObject *nv_create_circle(PyObject *self, PyObject *args) {
    nv_BodyType type;
    double x;
    double y;
    double angle;
    double radius;

    if (!PyArg_ParseTuple(
        args, "idddd",
        &type, &x, &y, &angle, &radius
    ))
        return NULL;

    PyObject *inst_args = Py_BuildValue("iidddd", type, 0, x, y, angle, radius);
    nv_BodyObject *obj = (nv_BodyObject *)PyObject_CallObject((PyObject *)&nv_BodyObjectType, inst_args);
    Py_DECREF(inst_args);
    return obj;
}

static PyObject *nv_create_rect(PyObject *self, PyObject *args) {
    nv_BodyType type;
    double x;
    double y;
    double angle;
    double width;
    double height;

    if (!PyArg_ParseTuple(
        args, "iddddd",
        &type, &x, &y, &angle, &width, &height
    ))
        return NULL;

    double w = width / 2.0;
    double h = height / 2.0;

    PyObject *inst_args = Py_BuildValue("iidddd((dd)(dd)(dd)(dd))", type, 1, x, y, angle, 0.0,
        -w, -h, w, -h, w, h, -w, h);

    nv_BodyObject *obj = (nv_BodyObject *)PyObject_CallObject((PyObject *)&nv_BodyObjectType, inst_args);
    Py_DECREF(inst_args);
    return obj;
}


#endif