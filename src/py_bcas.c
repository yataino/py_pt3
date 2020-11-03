#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stddef.h>
#include "structmember.h"
#include "datetime.h"
#include "py_bcas.h"

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

static PyObject* BcasError;
static PyMethodDef BcasMemberMethods[];
static PyModuleDef BcasModule;

static PyTypeObject BcasType;
static PyTypeObject BcasInitStatusType;
static PyTypeObject BcasPowerControlInfoType;
static PyTypeObject BcasEcmResultType;




static void
Bcas_Dealloc(PyObject *self)
{
    PyTypeObject *tp = Py_TYPE(self);
    BcasObject* bobj = (BcasObject*)self;
    if ( bobj->bcas )
    {
        bobj->bcas->release(bobj->bcas);
        bobj->bcas = NULL;
    }
    tp->tp_free(self);
}



static PyObject *
Bcas_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    BcasObject *self = (BcasObject*)type->tp_alloc(type, 0);
    B_CAS_CARD* bcas = create_b_cas_card();
    if ( !bcas )
    {
        Py_DECREF(self);
        return PyErr_NoMemory();
    }
    self->bcas = bcas;
    return (PyObject *) self;
}



static PyObject *
Bcas_init(PyObject *self, PyObject *args)
{
    BcasObject* bobj = (BcasObject*)self;
    bobj->bcas->init(bobj->bcas);
    Py_RETURN_NONE;
}



static PyObject *
Bcas_get_init_status(PyObject *self, PyObject *args)
{
    B_CAS_INIT_STATUS stat;
    BcasObject* bobj = (BcasObject*)self;
    int ret = bobj->bcas->get_init_status(bobj->bcas, &stat);
    if ( ret != 0 )
    {
        PyErr_SetString(BcasError, "B_CAS_CARD_ERROR_NOT_INITIALIZED");
        return NULL;
    }
    
    BcasInitStatusObject *status = PyObject_New(BcasInitStatusObject, &BcasInitStatusType);
    
    if ( status )
    {
        status->system_key = PyBytes_FromStringAndSize((char*)stat.system_key , ARRAY_LENGTH(stat.system_key));
        status->init_cbc   = PyBytes_FromStringAndSize((char*)stat.init_cbc   , ARRAY_LENGTH(stat.init_cbc  ));
        status->bcas_card_id = PyLong_FromLong(stat.bcas_card_id);
        status->card_status  = PyLong_FromLong(stat.card_status );
        status->ca_system_id = PyLong_FromLong(stat.ca_system_id);
        
        if ( !status->system_key   || !status->init_cbc     ||
             !status->bcas_card_id || !status->card_status  ||
             !status->ca_system_id  )
        {
            if ( status->system_key   ) Py_DECREF(status->system_key  );
            if ( status->init_cbc     ) Py_DECREF(status->init_cbc    );
            if ( status->bcas_card_id ) Py_DECREF(status->bcas_card_id);
            if ( status->card_status  ) Py_DECREF(status->card_status );
            if ( status->ca_system_id ) Py_DECREF(status->ca_system_id);
            Py_DECREF(status);
            return NULL;
        }
    }
    return (PyObject*)status;
}



static PyObject *
Bcas_get_id(PyObject *self, PyObject *args)
{
    B_CAS_ID bcasid;
    BcasObject* bobj = (BcasObject*)self;
    int ret = bobj->bcas->get_id(bobj->bcas, &bcasid);
    if ( ret != 0 )
    {
        PyErr_SetString(BcasError, "B_CAS_CARD_ERROR_NOT_INITIALIZED");
        return NULL;
    }
    
    if ( bcasid.count <= 0 )
    {
        PyErr_SetString(BcasError, "B_CAS_CARD_ERROR_NOT_INITIALIZED");
        return NULL;
    }
    
    PyObject* lst = PyList_New(bcasid.count);
    int i;
    for ( i = 0; i < bcasid.count; i++)
    {
        PyObject* item = PyLong_FromLongLong(bcasid.data[i]);
        if ( !item )
        {
            Py_DECREF(lst);
            return NULL;
        }
        PyList_Append(lst, item);
    }
    return lst;
}



static PyObject *
Bcas_get_pwr_on_ctrl(PyObject *self, PyObject *args)
{
    B_CAS_PWR_ON_CTRL_INFO ctlinfo;
    BcasObject* bobj = (BcasObject*)self;
    int ret = bobj->bcas->get_pwr_on_ctrl(bobj->bcas, &ctlinfo);
    if ( ret != 0 )
    {
        PyErr_SetString(BcasError, "B_CAS_CARD_ERROR_NOT_INITIALIZED");
        return NULL;
    }
    
    if ( ctlinfo.count <= 0 )
    {
        PyErr_SetString(BcasError, "B_CAS_CARD_ERROR_NOT_INITIALIZED");
        return NULL;
    }
    
    PyObject* lst = PyList_New(ctlinfo.count);
    
    int i;
    for (  i = 0; i < ctlinfo.count; i++ )
    {
        B_CAS_PWR_ON_CTRL* powerinfo = ctlinfo.data + i;
        BcasPowerControlInfoObject *info = PyObject_New(BcasPowerControlInfoObject, &BcasPowerControlInfoType);
        if ( info )
        {
            info->start_date           = PyDate_FromDate(powerinfo->s_yy, powerinfo->s_mm, powerinfo->s_dd);
            info->limit_date           = PyDate_FromDate(powerinfo->l_yy, powerinfo->l_mm, powerinfo->l_dd);
            info->hold_time            = PyLong_FromLong(powerinfo->hold_time            );
            info->broadcaster_group_id = PyLong_FromLong(powerinfo->broadcaster_group_id );
            info->network_id           = PyLong_FromLong(powerinfo->network_id           );
            info->transport_id         = PyLong_FromLong(powerinfo->transport_id         );
            if ( !info->start_date           || !info->limit_date           ||
                 !info->hold_time            || !info->broadcaster_group_id ||
                 !info->network_id           || !info->transport_id          )
            {
                Py_XDECREF(info->start_date           );
                Py_XDECREF(info->limit_date           );
                Py_XDECREF(info->hold_time            );
                Py_XDECREF(info->broadcaster_group_id );
                Py_XDECREF(info->network_id           );
                Py_XDECREF(info->transport_id         );
                Py_DECREF(info);
                Py_DECREF(lst);
                return NULL;
            }
        }
        PyList_Append(lst, (PyObject*)info);
    }
    return lst;
}



static PyObject *
Bcas_proc_ecm(PyObject *self, PyObject *args)
{
    B_CAS_ECM_RESULT ecm;
    unsigned char* src;
    int len;
    BcasObject* bobj = (BcasObject*)self;
    
    if ( !PyArg_ParseTuple(args, "s#:proc_ecm()", &src, &len) )
    {
        return NULL;
    }
    
    int ret = bobj->bcas->proc_ecm(bobj->bcas, &ecm, src, len);
    if ( ret != 0 )
    {
        PyErr_SetString(BcasError, "B_CAS_CARD_ERROR_NOT_INITIALIZED");
        return NULL;
    }
    
    BcasEcmResultObject *result = PyObject_New(BcasEcmResultObject, &BcasEcmResultType);
    if ( !result )
    {
        return NULL;
    }
    
    result->scramble_key = PyBytes_FromStringAndSize((char*)ecm.scramble_key ,ARRAY_LENGTH(ecm.scramble_key));
    result->return_code  = PyLong_FromUnsignedLong(ecm.return_code);
    if ( !result->scramble_key || !result->return_code )
    {
        Py_XDECREF(result->scramble_key);
        Py_XDECREF(result->return_code );
        Py_DECREF(result);
        return NULL;
    }
    return (PyObject*)result;
}



static PyObject *
Bcas_proc_emm(PyObject *self, PyObject *args)
{
    unsigned char* src;
    int len;
    BcasObject* bobj = (BcasObject*)self;
    
    if ( !PyArg_ParseTuple(args, "s#:proc_emm()", &src, &len) )
        return NULL;
    
    int ret = bobj->bcas->proc_emm(bobj->bcas, src, len);
    return PyLong_FromLong(ret);

}



PyMODINIT_FUNC
PyInit_bcas(void)
{
    PyDateTime_IMPORT;
    
    struct {
        const char* name;
        PyTypeObject* type;
    } types[] = {
                    {"Bcas"                 , &BcasType                },
                    {"BcasInitStatus"       , &BcasInitStatusType      },
                    {"BcasPowerControlInfo" , &BcasPowerControlInfoType},
                    {"BcasEcmResult"        , &BcasEcmResultType       },
                };
    
    int i;
    for ( i = 0; i < ARRAY_LENGTH(types); i++ )
    {
        if (PyType_Ready(types[i].type) < 0)
            return NULL;
    }
    
    PyObject *m = PyModule_Create(&BcasModule);
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
    
    BcasError = PyErr_NewException("bcas.Error", NULL, NULL);
    Py_XINCREF(BcasError);
    if (PyModule_AddObject(m, "Error", BcasError) < 0)
    {
        Py_XDECREF(BcasError);
        Py_CLEAR(BcasError);
        Py_DECREF(m);
        return NULL;
    }
    return m;
}



static PyModuleDef BcasModule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "bcas",
    .m_doc  = "bcas module",
    .m_size = -1,
};


//Bcas
static PyTypeObject BcasType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "bcas.Bcas",
    .tp_doc = "Bcas objects",
    .tp_basicsize = sizeof(BcasObject),
    .tp_itemsize  = 0,
    .tp_flags     = Py_TPFLAGS_DEFAULT,
    .tp_methods   = BcasMemberMethods,
    .tp_new       = Bcas_New,
    .tp_dealloc   = Bcas_Dealloc,
};



static PyMethodDef BcasMemberMethods[] = {
    {"init"           , Bcas_init           , METH_NOARGS , ""},
    {"get_init_status", Bcas_get_init_status, METH_NOARGS , ""},
    {"get_id"         , Bcas_get_id         , METH_NOARGS , ""},
    {"get_pwr_on_ctrl", Bcas_get_pwr_on_ctrl, METH_NOARGS , ""},
    {"proc_ecm"       , Bcas_proc_ecm       , METH_VARARGS, ""},
    {"proc_emm"       , Bcas_proc_emm       , METH_VARARGS, ""},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};



//BcasInitStatus
static PyMemberDef BcasInitStatusMembers[] = {
    {"system_key"  ,  T_OBJECT, offsetof(BcasInitStatusObject, system_key  ), READONLY, ""},
    {"init_cbc"    ,  T_OBJECT, offsetof(BcasInitStatusObject, init_cbc    ), READONLY, ""},
    {"bcas_card_id",  T_OBJECT, offsetof(BcasInitStatusObject, bcas_card_id), READONLY, ""},
    {"card_status" ,  T_OBJECT, offsetof(BcasInitStatusObject, card_status ), READONLY, ""},
    {"ca_system_id",  T_OBJECT, offsetof(BcasInitStatusObject, ca_system_id), READONLY, ""},
    {NULL} 
};



static void
BcasInitStatusDealloc(PyObject* obj)
{
    BcasInitStatusObject* self = (BcasInitStatusObject*)obj;
    PyTypeObject *tp = Py_TYPE(obj);
    Py_XDECREF(self->system_key     );
    Py_XDECREF(self->init_cbc       );
    Py_XDECREF(self->bcas_card_id   );
    Py_XDECREF(self->card_status    );
    Py_XDECREF(self->ca_system_id   );
    tp->tp_free(obj);
}



static PyTypeObject BcasInitStatusType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "bcas.BcasInitStatus",
    .tp_doc  = "BcasInitStatus objects",
    .tp_basicsize = sizeof(BcasInitStatusObject),
    .tp_itemsize  = 0,
    .tp_flags     = Py_TPFLAGS_DEFAULT,
    .tp_dealloc   = BcasInitStatusDealloc,
    .tp_members   = BcasInitStatusMembers,
};



//BcasEcmResult
static PyMemberDef BcasEcmResultMembers[] = {
    {"scramble_key" , T_OBJECT, offsetof(BcasEcmResultObject, scramble_key), READONLY, ""},
    {"return_code"  , T_OBJECT, offsetof(BcasEcmResultObject, return_code ), READONLY, ""},
    {NULL},
};



static void
BcasEcmResultDealloc(PyObject* obj)
{
    BcasEcmResultObject* self = (BcasEcmResultObject*)obj;
    PyTypeObject *tp = Py_TYPE(obj);
    Py_XDECREF(self->scramble_key);
    Py_XDECREF(self->return_code );
    tp->tp_free(obj);
}



static PyTypeObject BcasEcmResultType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "bcas.BcasEcmResult",
    .tp_doc  = "BcasEcmResult objects",
    .tp_basicsize = sizeof(BcasEcmResultObject),
    .tp_itemsize  = 0,
    .tp_flags     = Py_TPFLAGS_DEFAULT,
    .tp_dealloc   = BcasEcmResultDealloc,
    .tp_members   = BcasEcmResultMembers,
};


//BcasPowerControlInfo

static void
BcasPowerControlInfoDealloc(PyObject* obj)
{
    BcasPowerControlInfoObject* self = (BcasPowerControlInfoObject*)obj;
    PyTypeObject *tp = Py_TYPE(obj);
    Py_XDECREF(self->start_date           );
    Py_XDECREF(self->limit_date           );
    Py_XDECREF(self->hold_time            );
    Py_XDECREF(self->broadcaster_group_id );
    Py_XDECREF(self->network_id           );
    Py_XDECREF(self->transport_id         );
    tp->tp_free(obj);
}

static PyMemberDef BcasPowerControlInfoMembers[] = {
    { "start_date"          , T_OBJECT, offsetof(BcasPowerControlInfoObject, start_date          ), READONLY, ""},
    { "limit_date"          , T_OBJECT, offsetof(BcasPowerControlInfoObject, limit_date          ), READONLY, ""},
    { "hold_time"           , T_OBJECT, offsetof(BcasPowerControlInfoObject, hold_time           ), READONLY, ""},
    { "broadcaster_group_id", T_OBJECT, offsetof(BcasPowerControlInfoObject, broadcaster_group_id), READONLY, ""},
    { "network_id"          , T_OBJECT, offsetof(BcasPowerControlInfoObject, network_id          ), READONLY, ""},
    { "transport_id"        , T_OBJECT, offsetof(BcasPowerControlInfoObject, transport_id        ), READONLY, ""},
    {NULL}  /* Sentinel */
};


static PyTypeObject BcasPowerControlInfoType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "bcas.BcasPowerControlInfo",
    .tp_doc = "BcasPowerControlInfo objects",
    .tp_basicsize = sizeof(BcasPowerControlInfoObject),
    .tp_itemsize  = 0,
    .tp_flags     = Py_TPFLAGS_DEFAULT,
    .tp_dealloc   = BcasPowerControlInfoDealloc,
    .tp_members   = BcasPowerControlInfoMembers,
};

