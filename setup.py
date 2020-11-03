#!/usr/bin/env python3
# encoding: utf-8

from distutils.core import setup, Extension

ext_package = 'py_pt3'

bcas_module = Extension('bcas',
                     sources=['src/py_bcas.c',],
                     include_dirs=['/usr/local/include',],
                     libraries=['arib25',]
                     )
                     
arib25_module = Extension('arib25',
                     sources=['src/py_arib25.c',],
                     include_dirs=['/usr/local/include',],
                     libraries=['arib25',]
                     )             

pt3_module = Extension('pt3',
                     sources=['src/py_pt3.c',],
                     include_dirs=['/usr/local/include',],
                     )


setup (name = 'pt3',
       version = '0.1',
       description = 'This is a module for using libarib25 with python',
       ext_modules = [bcas_module, arib25_module, pt3_module],
       ext_package = ext_package,
       )
       

