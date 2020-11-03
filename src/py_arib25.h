
#ifndef _ARIB25_H
#define _ARIB25_H
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "arib25/arib_std_b25.h"


typedef struct {
    PyObject_HEAD
    ARIB_STD_B25* arib25;
    PyObject* bcas;
} Arib25Object;


typedef struct {
    PyObject_HEAD
    PyObject* program_number          ;
    PyObject* ecm_unpurchased_count   ;
    PyObject* last_ecm_error_code     ;
    PyObject* padding                 ;
    PyObject* total_packet_count      ;
    PyObject* undecrypted_packet_count;
} Arib25ProgramInfoObject;


extern PyTypeObject Arib25Type;
extern PyTypeObject Arib25ProgramInfoType;

#endif 

