/*

  This file is a part of the Nova Physics Engine
  project and distributed under the MIT license.

  Copyright © Kadir Aksoy
  https://github.com/kadir014/Nova-Physics

*/

#ifndef NV_PYTHON_SPACE_H
#define NV_PYTHON_SPACE_H

#include <Python.h>
#include "novaphysics/novaphysics.h"
#include "py_body.h"


/**
 * Space object interface
 */
typedef struct {
    PyObject_HEAD
    nv_Space *space;
    nv_Array *body_objects;
} nv_SpaceObject;

/**
 * Space object deallocater
 */
static void nv_SpaceObject_dealloc(nv_SpaceObject *self) {
    nv_Space_free(self->space);

    // Decrease reference of each body object in array
    for (size_t i = 0; i < self->body_objects->size; i++) {
        PyObject *body = (PyObject *)self->body_objects->data[i];
        Py_DECREF(body);
    }

    nv_Array_free(self->body_objects);

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
    self->space = nv_Space_new();
    self->body_objects = nv_Array_new();

    return 0;
}

/**
 * Space.step() method
 */
static PyObject *nv_SpaceObject_step(
    nv_SpaceObject *self,
    PyObject *args
) {
    double dt;
    int velocity_iters;
    int position_iters;
    int constraint_iters;
    int substeps;

    if (!PyArg_ParseTuple(
            args, "diiii",
            &dt,
            &velocity_iters,
            &position_iters,
            &constraint_iters,
            &substeps
    )) return NULL;

    nv_Space_step(
        self->space,
        dt,
        velocity_iters,
        position_iters,
        constraint_iters,
        substeps
    );

    // Update every body object
    for (size_t i = 0; i < self->body_objects->size; i++) {
        nv_Body *body = (nv_Body *)self->space->bodies->data[i];
        nv_BodyObject *body_object = (nv_BodyObject *)self->body_objects->data[i];
        Py_INCREF(body_object);
        
        body_object->position->x = body->position.x;
        body_object->position->y = body->position.y; 
        body_object->angle = body->angle; 
        body_object->radius = body->radius; 
    }

    Py_RETURN_NONE;
}

/**
 * Space.get_bodies() method
 * Returns nv_Space instance's bodies array as a Python tuple
 */
static PyObject *nv_SpaceObject_get_bodies(
    nv_SpaceObject *self,
    PyObject *Py_UNUSED(ignored)
) {
    PyObject *return_tup = PyTuple_New(self->body_objects->size);

    for (size_t i = 0; i < self->body_objects->size; i++) {
        nv_BodyObject *body = (nv_BodyObject *)self->body_objects->data[i];
        
        Py_INCREF(body);
        PyTuple_SET_ITEM(return_tup, i, body);
    }

    return return_tup;
}

static PyObject *nv_SpaceObject_add(
    nv_SpaceObject *self,
    PyObject *args
) {
    nv_BodyObject *body;

    if (!PyArg_ParseTuple(args, "O!", &nv_BodyObjectType, &body))
        return NULL;

    nv_Space_add(self->space, body->body);
    Py_INCREF(body);
    nv_Array_add(self->body_objects, body);

    Py_RETURN_NONE;
}

/**
 * Space object method interface
 */
static PyMethodDef nv_SpaceObject_methods[] = {
    {
        "step",
        (PyCFunction)nv_SpaceObject_step, METH_VARARGS,
        "Advance the simulation"
    },

    {
        "get_bodies",
        (PyCFunction)nv_SpaceObject_get_bodies, METH_NOARGS,
        "Get bodies"
    },

    {
        "add",
        (PyCFunction)nv_SpaceObject_add, METH_VARARGS,
        "Add body to space"
    },

    {NULL} // Sentinel
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


#endif