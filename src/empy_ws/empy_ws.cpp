#define PY_SSIZE_T_CLEAN
#include <Python.h>

static PyObject* empy_ws_connect(PyObject *self, PyObject *args)
{
  const char *address;

  if (!PyArg_ParseTuple(args, "s", &address))
    return NULL;

  printf("We should connect to %s\n", address);
  
  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef EmPyWs_Methods[] = {

    {"connect", empy_ws_connect, METH_VARARGS,
     "Execute a shell command."},

    {NULL, NULL, 0, NULL}
};

static PyModuleDef EmPyWsModule = {
    PyModuleDef_HEAD_INIT,
    "empy_ws", /* name of module */
    NULL,      /* module documentation, may be NULL */
    -1, /* size of per-interpreter state of the module,
           or -1 if the module keeps state in global variables. */
    EmPyWs_Methods
};

PyMODINIT_FUNC PyInit_empy_ws()
{
  return PyModule_Create(&EmPyWsModule);
}
