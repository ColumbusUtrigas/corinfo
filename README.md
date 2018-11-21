# corinfo
One-file C library for obtaining CPU, RAM and GPU info

The master hosted on github: https://github.com/ColumbusUtrigas/corinfo

## Using
You need only is to include **corinfo.h** in your project and you can use it!
Library does not require any compilation flags, other libraries or tons of code.

To get all info use

```c
struct corinfo info;
if (corinfo_GetInfo(&info) == -1) error();
```

## Documentation
To generate detailed documentation of library use

```sh
doxygen
```

