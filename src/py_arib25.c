#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stddef.h>
#include "structmember.h"

#include "py_arib25.h"
#include "py_bcas.h"

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

static PyObject* Arib25Error;
static PyMethodDef Arib25MemberMethods[];
static PyModuleDef Arib25module;


static void
Arib25_dealloc(PyObject *self)
{
    PyTypeObject* tp = Py_TYPE(self);
    Arib25Object* obj = (Arib25Object*)self;
    if ( obj->arib25 )
    {
        Py_XDECREF(obj->bcas);
        obj->arib25->release(obj->arib25);
        obj->arib25 = NULL;
    }
    tp->tp_free(self);
}



static PyObject *
Arib25_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Arib25Object *self = (Arib25Object *) type->tp_alloc(type, 0);
    ARIB_STD_B25* arib25 = create_arib_std_b25();
    if ( !arib25 )
    {
        Py_DECREF(self);
        return PyErr_NoMemory();
    }
    self->arib25 = arib25;
    return (PyObject *) self;
}



static PyObject *
Arib25_set_multi2_round(PyObject *self, PyObject *args)
{
    int32_t round;
    Arib25Object* obj = (Arib25Object*)self;

    if ( !PyArg_ParseTuple(args, "i:set_multi2_round()", &round) )
        return NULL;

    int ret = obj->arib25->set_multi2_round(obj->arib25, round);
    return PyLong_FromLong(ret);
}



static PyObject *
Arib25_set_strip(PyObject *self, PyObject *args)
{
    int32_t strip;
    Arib25Object* obj = (Arib25Object*)self;
    if ( !PyArg_ParseTuple(args, "i:set_strip()", &strip) )
        return NULL;

    int ret = obj->arib25->set_strip(obj->arib25, strip);
    return PyLong_FromLong(ret);
}


static PyObject *
Arib25_set_emm_proc(PyObject *self, PyObject *args)
{
    int32_t on;
    Arib25Object* obj = (Arib25Object*)self;
    if ( !PyArg_ParseTuple(args, "i:set_emm_proc()", &on) )
        return NULL;

    int ret = obj->arib25->set_emm_proc(obj->arib25, on);
    return PyLong_FromLong(ret);
}


static PyObject *
Arib25_set_b_cas_card(PyObject *self, PyObject *args)
{
    Arib25Object* obj = (Arib25Object*)self;
    PyObject* bcas = NULL;
    
    if ( !PyArg_ParseTuple(args, "O", &bcas) )
    {
        PyErr_SetString(Arib25Error, "args error");
        return NULL;
    }
    
    if ( obj->bcas != bcas )
    {
        Py_XDECREF(obj->bcas);
        obj->bcas = NULL;
    }
    
    int ret = 0;
    if ( bcas )
    {
        obj->bcas = bcas;
        Py_INCREF(obj->bcas);
        ret = obj->arib25->set_b_cas_card(obj->arib25, ((BcasObject*)bcas)->bcas);
    }
    return PyLong_FromLong(ret);
}


static PyObject *
Arib25_reset(PyObject *self, PyObject *args)
{
    Arib25Object* obj = (Arib25Object*)self;
    int ret = obj->arib25->reset(obj->arib25);
    return PyLong_FromLong(ret);
}


static PyObject *
Arib25_flush(PyObject *self, PyObject *args)
{
    Arib25Object* obj = (Arib25Object*)self;
    int ret = obj->arib25->flush(obj->arib25);
    return PyLong_FromLong(ret);
}



static PyObject *
Arib25_put(PyObject *self, PyObject *args)
{
    ARIB_STD_B25_BUFFER buf;
    Arib25Object* obj = (Arib25Object*)self;
    
    if ( !PyArg_ParseTuple(args, "s#:put()", &buf.data, &buf.size) )
        return NULL;

    int ret = obj->arib25->put(obj->arib25, &buf);
    return PyLong_FromLong(ret);
}



static PyObject *
Arib25_get(PyObject *self, PyObject *args)
{
    ARIB_STD_B25_BUFFER buf = {NULL,0};
    Arib25Object* obj = (Arib25Object*)self;

    int ret = obj->arib25->get(obj->arib25, &buf);
    if ( ret != 0 )
    {
        PyErr_SetString(Arib25Error, "ARIB_STD_B25_ERROR_INVALID_PARAM");
        return NULL;
    }
    return  PyBytes_FromStringAndSize((char*)buf.data, buf.size);
}


static PyObject *
Arib25_get_program_count(PyObject *self, PyObject *args)
{
    Arib25Object* obj = (Arib25Object*)self;
    int ret = obj->arib25->get_program_count(obj->arib25);
    return  PyLong_FromLong(ret);
}



static PyObject *
Arib25_get_program_info(PyObject *self, PyObject *args)
{
    int idx;
    ARIB_STD_B25_PROGRAM_INFO pinfo;
    Arib25Object* obj = (Arib25Object*)self;

    if ( !PyArg_ParseTuple(args, "i:get_program_info()", &idx) )
        return NULL;
    
    int ret = obj->arib25->get_program_info(obj->arib25, &pinfo, idx);
    if ( ret != 0 )
    {
        PyErr_SetString(Arib25Error, "ARIB_STD_B25_ERROR_INVALID_PARAM");
        return NULL;
    }
    
    Arib25ProgramInfoObject *info  = PyObject_New(Arib25ProgramInfoObject, &Arib25Type);
    if ( !info )
    {
        return NULL;
    }
    info->program_number           =  PyLong_FromLong(pinfo.program_number       );
    info->ecm_unpurchased_count    =  PyLong_FromLong(pinfo.ecm_unpurchased_count);
    info->last_ecm_error_code      =  PyLong_FromLong(pinfo.last_ecm_error_code  );
    info->padding                  =  PyLong_FromLong(pinfo.padding              );
    info->total_packet_count       =  PyLong_FromLongLong(pinfo.total_packet_count      );
    info->undecrypted_packet_count =  PyLong_FromLongLong(pinfo.undecrypted_packet_count);
    return  (PyObject*)info;
}




PyMODINIT_FUNC
PyInit_arib25(void)
{
    struct {
        const char* name;
        PyTypeObject* type;
    } types[] = {
                    {"Arib25"           , &Arib25Type           },
                    {"Arib25ProgramInfo", &Arib25ProgramInfoType},
                };
    int i;
    for ( i = 0; i < ARRAY_LENGTH(types); i++ )
    {
        if (PyType_Ready(types[i].type) < 0)
            return NULL;
    }
    
    PyObject *m = PyModule_Create(&Arib25module);
    if (m == NULL)
        return NULL;
    
    for ( i = 0; i < ARRAY_LENGTH(types); i++ )
    {
        Py_INCREF(types[i].type);
        if (PyModule_AddObject(m, types[i].name, (PyObject*)types[i].type) < 0) 
        {
            int j;
            for ( j = 0; j <= i; j++ )
            {
                Py_DECREF(types[j].type);
            }
            Py_DECREF(m);
            return NULL;
        }
    }
    
    Arib25Error = PyErr_NewException("arib25.Error", NULL, NULL);
    Py_XINCREF(Arib25Error);
    if (PyModule_AddObject(m, "Error", Arib25Error) < 0)
    {
        Py_XDECREF(Arib25Error);
        Py_CLEAR(Arib25Error);
        Py_DECREF(m);
        return NULL;
    }
    return m;
}


static PyModuleDef Arib25module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "Arib25",
    .m_doc = "Arib25",
    .m_size = -1,
};



PyTypeObject Arib25Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "arib25.Arib25",
    .tp_doc       = "Arib25 objects",
    .tp_basicsize = sizeof(Arib25Object),
    .tp_itemsize  = 0,
    .tp_flags     = Py_TPFLAGS_DEFAULT,
    .tp_methods   = Arib25MemberMethods,
    .tp_new       = Arib25_new,
    .tp_dealloc   = Arib25_dealloc,
};





static PyMethodDef Arib25MemberMethods[] = {
    {"set_multi2_round"  , Arib25_set_multi2_round  , METH_VARARGS, "" },
    {"set_strip"         , Arib25_set_strip         , METH_VARARGS, "" },
    {"set_emm_proc"      , Arib25_set_emm_proc      , METH_VARARGS, "" },
    {"set_b_cas_card"    , Arib25_set_b_cas_card    , METH_VARARGS, "" },
    {"reset"             , Arib25_reset             , METH_NOARGS , "" },
    {"flush"             , Arib25_flush             , METH_VARARGS, "" },
    {"put"               , Arib25_put               , METH_VARARGS, "" },
    {"get"               , Arib25_get               , METH_NOARGS , "" },
    {"get_program_count" , Arib25_get_program_count , METH_VARARGS, "" },
    {"get_program_info"  , Arib25_get_program_info  , METH_VARARGS, "" },
    {NULL, NULL, 0, NULL}        /* Sentinel */
};



static PyMemberDef Arib25ProgramInfoMembers[] = {
    {"program_number"          , T_OBJECT, offsetof(Arib25ProgramInfoObject, program_number          ), READONLY, ""},
    {"ecm_unpurchased_count"   , T_OBJECT, offsetof(Arib25ProgramInfoObject, ecm_unpurchased_count   ), READONLY, ""},
    {"last_ecm_error_code"     , T_OBJECT, offsetof(Arib25ProgramInfoObject, last_ecm_error_code     ), READONLY, ""},
    {"padding"                 , T_OBJECT, offsetof(Arib25ProgramInfoObject, padding                 ), READONLY, ""},
    {"total_packet_count"      , T_OBJECT, offsetof(Arib25ProgramInfoObject, total_packet_count      ), READONLY, ""},
    {"undecrypted_packet_count", T_OBJECT, offsetof(Arib25ProgramInfoObject, undecrypted_packet_count), READONLY, ""},
    {NULL},
};



static void
Arib25ProgramInfoDealloc(PyObject* obj)
{
    Arib25ProgramInfoObject* self = (Arib25ProgramInfoObject*)obj;
    PyTypeObject *tp = Py_TYPE(obj);
    Py_XDECREF(self->program_number           );
    Py_XDECREF(self->ecm_unpurchased_count    );
    Py_XDECREF(self->last_ecm_error_code      );
    Py_XDECREF(self->padding                  );
    Py_XDECREF(self->total_packet_count       );
    Py_XDECREF(self->undecrypted_packet_count );
    tp->tp_free(obj);
}



PyTypeObject Arib25ProgramInfoType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "arib25.Arib25ProgramInfo",
    .tp_doc  = "Arib25ProgramInfo objects",
    .tp_basicsize = sizeof(Arib25ProgramInfoObject),
    .tp_itemsize  = 0,
    .tp_flags     = Py_TPFLAGS_DEFAULT,
    .tp_dealloc   = Arib25ProgramInfoDealloc,
    .tp_members   = Arib25ProgramInfoMembers,
};


