#ifndef _BCAS_H
#define _BCAS_H
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "arib25/b_cas_card.h"


typedef struct {
    PyObject_HEAD
    B_CAS_CARD* bcas;
} BcasObject;

typedef struct {
    PyObject_HEAD
    PyObject* system_key;
    PyObject* init_cbc;
    PyObject* bcas_card_id;
    PyObject* card_status;
    PyObject* ca_system_id;
} BcasInitStatusObject;

typedef struct {
    PyObject_HEAD
    PyObject* start_date;
    PyObject* limit_date;
    PyObject* hold_time; 
    PyObject* broadcaster_group_id;
    PyObject* network_id;
    PyObject* transport_id;
} BcasPowerControlInfoObject;

typedef struct {
    PyObject_HEAD
    PyObject* scramble_key;
    PyObject* return_code;
} BcasEcmResultObject;


#endif 

