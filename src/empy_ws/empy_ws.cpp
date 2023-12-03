#define PY_SSIZE_T_CLEAN
#include <Python.h>

#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>
#endif

#include "pool.h"

// Module methods
static PyObject* empy_ws_connect(PyObject *self, PyObject *args);
static PyObject* empy_ws_disconnect(PyObject *self, PyObject *args);

// Module definitions
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

// Implementations

namespace empy_ws {
  class web_socket_data {
  public:
    web_socket_data(web_socket_data &&o) = delete;
    web_socket_data(web_socket_data const &o) = delete;

    web_socket_data(PyObject *pOnOpenFunc, PyObject *pOnMessageFunc, PyObject *pOnCloseFunc, PyObject *pOnErrorFunc)
      : m_pOnOpen(pOnOpen)
      , m_pOnMessage(pOnMessageFunc)
      , m_pOnClose(pOnCloseFunc)
      , m_pOnError(pOnErrorFunc)
    {
      emscripten_websocket_set_onopen_callback(ws, NULL, &web_socket_data::open_callback);
      emscripten_websocket_set_onmessage_callback(ws, NULL, &web_socket_data::message_callback);
      emscripten_websocket_set_onclose_callback(ws, NULL, &web_socket_data::close_callback);
      emscripten_websocket_set_onerror_callback(ws, NULL, &web_socket_data::error_callback);

      Py_INCREF(m_pOnOpen);
      Py_INCREF(m_pOnMessage);
      Py_INCREF(m_pOnClose);
      Py_INCREF(m_pOnError);
    }

    ~web_socket_data()
    {
      Py_DECREF(m_pOnOpen);
      Py_DECREF(m_pOnMessage);
      Py_DECREF(m_pOnClose);
      Py_DECREF(m_pOnError);
    }

    /// Call the on open python callback
    bool on_open()
    {
      puts("On Open\n");

      PyObject *arglist = Py_BuildValue("");
      PyObject *result  = PyObject_CallObject(my_callback, arglist);

      Py_DECREF(arglist);
      if (result == NULL)
        return false;

      Py_DECREF(result);
      return true;
    }

    /// Call the on message python callback
    bool on_message(uint8_t *data, uint32_t numBytes, bool isText)
    {
      puts("On Message\n");
      return false;
    }

    /// Call the on error python callback
    bool on_error()
    {
      puts("On Close\n");
      return false;
    }

    /// Call the on close python callback
    bool on_close(uint16_t code, bool wasClean, char *reason)
    {
      puts("On Error\n");
      return false;
    }

  private:
    PyObject *m_pOnOpen = nullptr;
    PyObject *m_pOnMessage = nullptr;
    PyObject *m_pOnClose = nullptr;
    PyObject *m_pOnError = nullptr;

    static bool open_callback(int eventType, const EmscriptenWebSocketOpenEvent *ev, void *userData) {
      return ((web_socket_data*)userData)->on_open();
    }

    static bool message_callback(int eventType, const EmscriptenWebSocketMessageEvent *ev, void *userData) {
      return ((web_socket_data*)userData)->on_message(ev->data, ev->numBytes, ev->isText);
    }

    static bool close_callback(int eventType, const EmscriptenWebSocketCloseEvent *ev, void *userData) {
      return ((web_socket_data*)userData)->on_close(ev->code, ev->wasClean, ev->reason);
    }

    static bool error_callback(int eventType, const EmscriptenWebSocketErrorEvent *ev, void *userData) {
      return ((web_socket_data*)userData)->on_error();
    }
  };

  static std::map<int, std::unique_ptr<web_socket_data>> sockets;
}

PyObject* empy_ws_connect(PyObject *self, PyObject *args)
{
  const char *address;
  PyObject* *pOnOpen, pOnMessage, pOnClose, pOnError;

  if (!PyArg_ParseTuple(args, "sOOOO", &address, &pOnOpen, &pOnMessage, &pOnClose, &pOnError))
    return NULL;

  if (!PyCallable_Check(pOnOpen) ||
      !PyCallable_Check(pOnMessage) ||
      !PyCallable_Check(pOnClose) ||
      !PyCallable_Check(pOnError)) {
    PyErr_SetString(PyExc_TypeError, "parameter must be callable");
    return NULL;
  }

  printf("Connecting to %s\n", address);
  
  EmscriptenWebSocketCreateAttributes attribs = {
      address,
      NULL,
      EM_TRUE
  };

  EMSCRIPTEN_WEBSOCKET_T ws = emscripten_websocket_new(&attribs);

  if (ws < 0) {
    printf("Failed to connect to %s\n");
    // TODO: Throw python exception
    return NULL; // An error occurred.
  }

  sockets.insert(ws, std::make_unique<empy_ws::web_socket_data>(pOnOpen, pOnMessage, pOnClose, pOnError));

  return PyLong_FromLong(ws);
}

PyObject* empy_ws_disconnect(PyObject *self, PyObject *args)
{
  const char *address;

  if (!PyArg_ParseTuple(args, "s", &address))
  {
    // TODO: Throw python exception
    return NULL;
  }

  printf("We should connect to %s\n", address);

  Py_INCREF(Py_None);
  return P;
}
