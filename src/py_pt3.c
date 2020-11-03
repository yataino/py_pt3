#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>


typedef struct  {
    int freq; // 周波数テーブル番号
    int slot; // スロット番号／加算する周波数
} PYPT3_FREQUENCY;


#define PY_MAX_READ_SIZE       (188 * 87) /* 188*87=16356 splitterが188アライメントを期待しているのでこの数字とする*/
#define PY_SET_CHANNEL          _IOW(0x8D, 0x01, PYPT3_FREQUENCY)
#define PY_START_REC            _IO( 0x8D, 0x02)
#define PY_STOP_REC             _IO( 0x8D, 0x03)
#define PY_GET_SIGNAL_STRENGTH  _IOR(0x8D, 0x04, int *)
#define PY_LNB_ENABLE           _IOW(0x8D, 0x05, int)
#define PY_LNB_DISABLE          _IO( 0x8D, 0x06)

static PyGetSetDef  PyPt3_GetSetters[];
static PyMethodDef  PyPt3_MemberMethods[];
static PyTypeObject PyPt3Type;
static PyModuleDef  PyPt3Module;
static PyObject*    PyPt3Error;


typedef struct {
    PyObject_HEAD
    int fd                  ;
    int buffer_size         ;
    int set_channel         ;
    int start_rec           ;
    int stop_rec            ;
    int get_signal_strength ;
    int lnb_enable          ;
    int lnb_disable         ;
} PyPt3Object;



static void 
PyPt3Err_SetString(const char* fmt, ...)
{
    char buff[512] = {0};
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, sizeof(buff), fmt, args);
    PyErr_SetString(PyPt3Error, buff);
    va_end(args);
}



static void
PyPt3_Dealloc(PyObject *self)
{
    PyTypeObject *tp = Py_TYPE(self);
    PyPt3Object* obj = (PyPt3Object*)self;
    if (obj->fd != -1 )
    {
        close(obj->fd);
        obj->fd = -1;
    }
    tp->tp_free(self);
}



static PyObject *
PyPt3_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyPt3Object *self = (PyPt3Object *) type->tp_alloc(type, 0);
    if ( self )
    {
        self->fd                  = -1;
        self->set_channel         = PY_SET_CHANNEL        ;
        self->start_rec           = PY_START_REC          ;
        self->stop_rec            = PY_STOP_REC           ;
        self->get_signal_strength = PY_GET_SIGNAL_STRENGTH;
        self->lnb_enable          = PY_LNB_ENABLE         ;
        self->lnb_disable         = PY_LNB_DISABLE        ;
        self->buffer_size           = PY_MAX_READ_SIZE      ;
    }
    return (PyObject *) self;
}



static PyObject *
PyPt3_SetChannel(PyObject *self, PyObject *args, PyObject *kwds)
{
    PyPt3Object* obj = (PyPt3Object*)self;
    if ( obj->fd == -1 )
    {
        PyPt3Err_SetString("set channel: device not open");
        return NULL;
    }
    else
    {
        PYPT3_FREQUENCY channel = {-1,-1};
        int* freq = &channel.freq;
        int* slot = &channel.slot;
        static char *kwlist[] = {"freq", "slot" , NULL};
        
        if ( !PyArg_ParseTupleAndKeywords(args, kwds, "ii:set_channel()", kwlist, freq, slot) )
            return NULL;

        errno = 0;
        if ( ioctl(obj->fd, obj->set_channel, &channel) == -1 )
        {
            PyPt3Err_SetString("set channel: ioctl error [%d]", errno);
            return NULL;
        }
    }
    Py_RETURN_NONE;
}



static PyObject *
PyPt3_EnableLNB(PyObject *self, PyObject *args)
{
    PyPt3Object* obj = (PyPt3Object*)self;
    if ( obj->fd == -1 )
    {
        PyPt3Err_SetString("enablde LNB: not open");
        return NULL;
    }
    else
    {
        int lnb ;
        if ( PyArg_ParseTuple(args, "i:enable_lnb()", &lnb) )
        {
            errno = 0;
            if ( ioctl(obj->fd, obj->lnb_enable, lnb) == -1 )
            {
                PyPt3Err_SetString("enablde LNB: ioctl error [%d]", errno);
                return NULL;
            }
        }
        else
        {
            PyPt3Err_SetString("enablde LNB: args error");
            return NULL;
        }
    }
    Py_RETURN_NONE;
}



static PyObject *
PyPt3_DisableLNB(PyObject *self, PyObject *args)
{
    PyPt3Object* obj = (PyPt3Object*)self;
    if ( obj->fd == -1 )
    {
        PyPt3Err_SetString("disable LNB: not open");
        return NULL;
    }
    else
    {
        errno = 0;
        if ( ioctl(obj->fd, obj->lnb_disable,0) == -1 )
        {
            PyPt3Err_SetString("disable LNB: ioctl error [%d]", errno);
            return NULL;
        }
    }
    Py_RETURN_NONE;
}



static PyObject *
PyPt3_Start(PyObject *self, PyObject *args)
{
    PyPt3Object* obj = (PyPt3Object*)self;
    if ( obj->fd == -1 )
    {
        PyPt3Err_SetString("start: not open");
        return NULL;
    }
    else
    {
        errno = 0;
        if ( ioctl(obj->fd, obj->start_rec, 0) == -1 )
        {
            PyPt3Err_SetString("start: ioctl error [%d]", errno);
            return NULL;
        }
    }
    Py_RETURN_NONE;
}



static PyObject *
PyPt3_Stop(PyObject *self, PyObject *args)
{
    PyPt3Object* obj = (PyPt3Object*)self;
    if ( obj->fd == -1 )
    {
        PyPt3Err_SetString("stop: not open");
        return NULL;
    }
    else
    {
        errno = 0;
        if ( ioctl(obj->fd, obj->stop_rec, 0) == -1 )
        {
            PyPt3Err_SetString("stop: ioctl Error [%d]", errno);
            return NULL;
        }
    }
    Py_RETURN_NONE;
}


static PyObject *
pt3_open(PyPt3Object *obj, PyObject *args, PyObject *kwds)
{
    const char* device = NULL;
    int size = PY_MAX_READ_SIZE;
    
    static char *kwlist[] = {"device", "buffer_size" , NULL};
    if ( !PyArg_ParseTupleAndKeywords(args, kwds, "s|i:open()", kwlist, &device, &size) )
        return NULL;
    
    if ( size <= 0 )
    {
        PyPt3Err_SetString("open: buffer_size error");
        return NULL;
    }
    
    if ( device )
    {
        errno = 0;
        int fd = open(device, O_RDONLY);
        if ( fd >= 0 )
        {
            obj->fd = fd;
            obj->buffer_size = size;
        }
        else
        {
            PyPt3Err_SetString("oprn: error [%d]", errno);
            return NULL;
        }
    }
    return (PyObject*)obj;
}



static PyObject *
PyPt3_Open(PyObject *self, PyObject *args, PyObject *kwds)
{
    PyPt3Object* obj = (PyPt3Object*)self;
    if (obj->fd != -1 )
    {
        close(obj->fd);
        obj->fd = -1;
    }
    pt3_open(obj, args, kwds);
    Py_RETURN_NONE;
}



static PyObject *
PyPt3ModuleOpen(PyObject *self, PyObject *args, PyObject *kwds)
{
    PyPt3Object* obj = (PyPt3Object*)PyPt3_New(&PyPt3Type, NULL, NULL);
    return pt3_open(obj, args, kwds);
}



static PyObject *
PyPt3_Close(PyObject *self, PyObject *args)
{
    PyPt3Object* obj = (PyPt3Object*)self;
    if (obj->fd != -1 )
    {
        close(obj->fd);
        obj->fd = -1;
    }
    Py_RETURN_NONE;
}



static PyObject *
PyPt3_Read(PyObject *self, PyObject *args)
{
    PyPt3Object* obj = (PyPt3Object*)self;

    if ( obj->fd == -1 )
    {
        PyPt3Err_SetString("read: not open");
        return NULL;
    }

    int size = obj->buffer_size;
    if ( !PyArg_ParseTuple(args, "|i:read()", &size) )
        return NULL;

    PyObject* v = PyBytes_FromStringAndSize(NULL, size);
    if ( !v )
    {
        PyPt3Err_SetString("read: alocate error");    
        return NULL;
    }
    
    errno = 0;
    int readed_size = read(obj->fd , PyBytes_AS_STRING(v), size);
    if ( readed_size < 0 )
    {
        Py_DECREF(v);
        PyPt3Err_SetString("read: io error[%d]", errno);
        return NULL;
    }

    if ( size != readed_size )
        _PyBytes_Resize(&v, readed_size);
    return v;
}



static PyObject *
PyPt3_SetRequest(PyObject *self, PyObject *args, PyObject *kwds)
{
    const char* name = NULL;
    int type;
    int number;
    PyPt3Object* obj = (PyPt3Object*)self;
    
    static char *kwlist[] = {"name", "type", "number" , NULL};
    if ( !PyArg_ParseTupleAndKeywords(args, kwds, "sii:set_request()", kwlist, &name, &type, &number) )
        return NULL;

    if      ( !strcmp(name, "set_channel"         )){ obj->set_channel          =  _IOW(type, number, PYPT3_FREQUENCY);}
    else if ( !strcmp(name, "start_rec"           )){ obj->start_rec            =  _IO( type, number)                 ;}
    else if ( !strcmp(name, "stop_rec"            )){ obj->stop_rec             =  _IO( type, number)                 ;}
    else if ( !strcmp(name, "get_signal_strength" )){ obj->get_signal_strength  =  _IOR(type, number, int *)          ;}
    else if ( !strcmp(name, "lnb_enable"          )){ obj->lnb_enable           =  _IOW(type, number, int)            ;}
    else if ( !strcmp(name, "lnb_disable"         )){ obj->lnb_disable          =  _IO( type, number)                 ;}
    else
    {
        PyPt3Err_SetString("set request: args error [%s]", name);
        return NULL;
    }
    Py_RETURN_NONE;
}



static PyObject *
PyPt3_Closed(PyObject *self, void *context)
{
    PyPt3Object* obj = (PyPt3Object*)self;
    int closed = 0;
    if (obj->fd == -1 )
        closed = 1;
   return PyBool_FromLong(closed);
}



static PyObject *
PyPt3_Fileno(PyObject *self, void *context)
{
    PyPt3Object* obj = (PyPt3Object*)self;
    return PyLong_FromLong(obj->fd);
}



static PyObject *
PyPt3_Enter(PyObject *self, PyObject *args)
{
    Py_INCREF(self);
    return self;
}



static PyObject *
PyPt3_Exit(PyObject *self, PyObject *args)
{
    PyPt3_Close(self, NULL);
    Py_RETURN_NONE;
}



PyMODINIT_FUNC
PyInit_pt3(void)
{
    if (PyType_Ready(&PyPt3Type) < 0)
        return NULL;

    PyObject *m = PyModule_Create(&PyPt3Module);
    if (m == NULL)
        return NULL;

    Py_INCREF(&PyPt3Type);
    if (PyModule_AddObject(m, "Pt3", (PyObject*)&PyPt3Type) < 0)
    {
        Py_DECREF(m);
        return NULL;
    }
    
    PyPt3Error = PyErr_NewException("pt3.Error", NULL, NULL);
    Py_XINCREF(PyPt3Error);
    if (PyModule_AddObject(m, "Error", PyPt3Error) < 0)
    {
        Py_XDECREF(PyPt3Error);
        Py_CLEAR(PyPt3Error);
        Py_DECREF(m);
        return NULL;
    }
    return m;
}



static PyTypeObject PyPt3Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "pt3.Pt3"           ,
    .tp_doc       = "Pt3 object"        ,
    .tp_basicsize = sizeof(PyPt3Object) ,
    .tp_itemsize  = 0                   ,
    .tp_flags     = Py_TPFLAGS_DEFAULT  ,
    .tp_methods   = PyPt3_MemberMethods ,
    .tp_new       = PyPt3_New           ,
    .tp_dealloc   = PyPt3_Dealloc       ,
    .tp_getset    = PyPt3_GetSetters    ,
};


static PyGetSetDef PyPt3_GetSetters[] = {
    {"closed", PyPt3_Closed, NULL, NULL},
    {"fileno", PyPt3_Fileno, NULL, NULL},
    {NULL}
};


static PyMethodDef PyPt3_MemberMethods[] = {
    {"set_channel", (PyCFunction)PyPt3_SetChannel, METH_VARARGS | METH_KEYWORDS , ""},
    {"enable_lnb" , PyPt3_EnableLNB              , METH_VARARGS                 , ""},
    {"disable_lnb", PyPt3_DisableLNB             , METH_NOARGS                  , ""},
    {"start"      , PyPt3_Start                  , METH_NOARGS                  , ""},
    {"stop"       , PyPt3_Stop                   , METH_NOARGS                  , ""},
    {"open"       , (PyCFunction)PyPt3_Open      , METH_VARARGS | METH_KEYWORDS , ""},
    {"close"      , PyPt3_Close                  , METH_NOARGS                  , ""},
    {"read"       , PyPt3_Read                   , METH_VARARGS                 , ""},
    {"set_request", (PyCFunction)PyPt3_SetRequest, METH_VARARGS | METH_KEYWORDS , ""},
    {"__enter__"  , PyPt3_Enter                  , METH_NOARGS                  , ""},
    {"__exit__"   , PyPt3_Exit                   , METH_VARARGS                 , ""},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};


static PyMethodDef PyPt3ModuleMethods[] = {
    {"open"       , (PyCFunction)PyPt3ModuleOpen, METH_VARARGS | METH_KEYWORDS , ""},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};



static PyModuleDef PyPt3Module = {
    PyModuleDef_HEAD_INIT,
    .m_name    = "pt3",
    .m_doc     = "pt3",
    .m_size    = -1,
    .m_methods = PyPt3ModuleMethods,
};

