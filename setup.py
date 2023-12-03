from setuptools import setup, Extension

setup(
    ext_modules= [
      Extension(
        "empy_ws",
        sources=["./src/empy_ws/empy_ws.cpp"]
      )
    ]
)
