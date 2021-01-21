# QPROP v3.91

QPROP (short for Quantum Propagation) is a tool for performing single active electron (SAE) simulations of quantum systems in intense laser fields.

This version is built upon v3.2 and aims to improve usability and sustainability of the project.

The original (C) copyright goes to Bauer D and Koval P, Heidelberg (2005), Tulsky V A (2020)

# Installation

## Prerequisites for using the package:

- GNU Scientific Library (gsl versions 1.16 and 2.0 have been tested)
- GNU C++ compiler supporting the C++11 standard (Versions 4.7.2, 4.8.4 and 5.3.0 have been tested)
- (Optional) Open MPI (parallelized version of spectrum evaluation)

### Easy install on Mac OS:

- `brew install gcc`
- `brew install gsl`
- (optional) `brew install openmpi`

# Building

Several examples are provided in the `projects/` directory, all of which can be be compiled and run using the main Makefile using the command:

`make proj=projectname all`

where `projectname` is the name of one of the folders inside `projects/`.

Ann additional script called `make-all` can be used to build all the projects at once.
It takes one argument which is being passed to make.

# Running using 'run' routine

Among the examples you will find a project named `template` which can be used for creating own projects. Let's build & run the `template` project:

`run -make all proj=template`

The command will run all the routines automatically.
After a few seconds results can be found in your home folder under:

`~/Results/template`.

To customize how the project is run modify `config.hh` and/or `config.param` files, which posses many self-explanatory comments. For more see `Customization` paragraph.

# What's inside?

- `base/` contains the source files of the QPROP library along with a Makefile to build it. It will be called automatically when using the main `Makefile` through `make`.

- `plots/` contains Python scripts for visualising the qprop-produced results.

- `projects/` holds predefined projects showcasing various functionality and a special `template` project designed for customization.

- `remote/` contains a set of four predefined scripts for handling remote (ssh) code: distribution &execution (`uploadrun`), inspecting code execution (`view`), downloading the results (`download`) or removing them (`delete`).

- `wip` - ignore it for now (work in progress, aka trash-bin).

- `bk` helper script for running jobs in the background and creating a log file with the name of the command passed.
  Used by `remote` script.

- `common-variables-for-make.mk` holds compiler options. Modify if:

  - you want to change the compiler
  - your gsl or MPI libraries/includes reside in different directories
  - you encounter compilation errors

- `commonvars.hh` holds globals, produces logs and processes options. Used in all the standard routines (`im`, `re`, etc.).

- `default-config.param` contains the default configuration for all projects. Can be adjusted for individual needs.

- `im.cc` is the shared imaginary time propagation code used for finding (`max-n-qnumber` - `l-qnumber`) eigenstates of given `l-qnumber` number (quantum l number).

- `isurfv.cc` is the iSURFV routine code. Nothing essential has changed here since v3.2.
- `Makefile` main makefile. See `Running` paragraph.
- `potentials.hh` holds (shared) predefined vector, scalar and imaginary potentials.
- `re.cc` is the shared real time propagation code.
- `remote` can be used to build and run on a remote machine through ssh. Takes the following options:
  - `-host` (ip address or alias)
  - `-user` (username to use for ssh login)
  - `-action` (defines what will the script do. Pass `u` to upload the source, `r` to run a project, `v` for viewing evaluation progress, `d` to download results or `c` to clean the remote from the source)
  - After those (required) options the `run` file options (see below) and the regular `im/re/isurfv/tsurff` options can follow (in this order).
- `run` can be used to build and run any project. Takes the following options:
  - `-proj=` (for passing the project name, just like in regular make)
  - `-np` (number of processor for parallel execution)
  - `-make` (for making the binaries before run)
  - `-special` (for customization - will execute `projects/projectname/run` instead its own routines)
  - `-from` and `-upto` (for setting up limits to routines run, ex. `-from im -upto re` will run only `./im` and `./re`)
  - `-dryrun` (will present what will be run, but without running).
  - After those (mostly optional) options the regular `im/re/isurfv/tsurff` options can follow and will be pass to all routines.
- `isurfv.cc` is the iSURFV/tSURFF routine code. Nothing essential has changed here since v3.2.

# Customization

Copy and rename the `template` project folder. Look inside `config.hh` and `config.param`, both hold many comments on how to efficiently customize your project.
Most often you'll need to modify the `config.param` file and/or the active preprocessor directives from `config.hh` file together with the `configPulse()` function.

By design all projects share the _files_ from the main directory directory.
The configurations specific to each project reside in their project folders.
Custom binaries are compiled to their project folders.

The main rules for creating new project efficiently can be stated as:

1. Preprocessor directives in `config.hh` should declare (at compile time) which subroutines should be run, ex. HHG, populations, electron momentum spectra etc.
2. Values from `config.param` define the parameters with which the routines should be run.
3. Occasionally, standard input (stdin) can be used for overwriting the above parameters by _defining_ (it should always stay _declared_) `processCustomOptions()` function in `config.hh`.
4. Extra potential/laser parameters should be defined in `config.hh` or through custom stdin input parsed with `processCustomOptions()`.
5. If you need to modify files other than the files in the project folder consider making a contribution, i.e., a pull request. Base directory should be good as it is, but if you find any improvements please let us know.

# Changes compared with v3.2

## Main ones:

- Renaming. It is an esthetic change, but important to understand first. Most important changes:

  - `imag_prop` -> `im`
  - `real_prop` -> `re`
  - `tsurff` and `isurfv` has been left without a change for now.
  - All `*.param` files have been merged into a single `config.param` file per project.
  - Some of the `config.param` variables have also been renamed, see comments in `default-config.param` to understand what does what.
  - Since the `im` routine allows for finding excited states since v3.1 the output files are now named consistently with quantum numbers n, l, m. Support for superpositions is being considered.

- `tsurff_mpi.cc` file is removed and unified to `tsurff.cc` which can be compiled to `tsurff` or `tsurff-mpi` with the default Makefile.
- Global namespace. Since most of the parameters need to be constantly piped between `im`, `re`, `isurfv` and `tsurff` all the essential variables are now globals fetched from `commonvars.hh`. This makes less room for error.
- Logging. New function `logAdd()` allows for logging simultaneously to the console and the log files whenever `x-log-inverval` is greater than zero. (`x` is the short name for the routine). Standard `grid` object related logs have been moved to `logSilently()`, which I believe will be deprecated in a future release when the new logs become more polished.
- Projects. The standard routines `im.cc`, `re.cc`, `isurfv.cc` and `tsurff.cc` are now shared, which means no more wasteful copying. Customizations are now stored in individual folders inside `projects` folder. Projects are also compiled using a single, common Makefile located in main directory which will output the executables to their project's folder.
  - Motivation: If a bug will be found in main source-code all projects will be fixed at once.
  - Requirements: Every new project needs a `config.hh` and `config.param` file together with a bunch of required functions run by the routines as exampled in `template` project.
- Defaults and parameter overrides. By introducing `default-config.param` new projects need less initial configuration. The default configurations will be drawn from this file on init in every routine
  - The `config.param` of example projects, such as `attoclock`, could have had less parameters, but we retain them to protect the projects from modifying the defaults)
  - The "general order" in which variables are initiated or overridden goes as follows
    `default-config.param => config.param => processOptions() => processCustomOptions()`
    where processOptions() uses unix `getopt_long` to recognize long and short options used for quick change of variable values.
    - For example `nuclear-charge` can be changed by simply passing `-Z newvalue` or `--nuclear-charge new value` options (`newvalue` neads to be a number) to any of the routines.
    - All defined options can be displayed with `-h` option.
    - An important (useful) exception to the "general order" is when config/default config files are passed using `-C newconfig.param` or `-D newdefault-config.param` respectively; in this case only the new config files are loaded before other options and user (custom) options are parsed.
    - All this information override will be stored in the default `*.log` files. Which, I think is quite important.
- By default results will be located in `~/Results/projectname`, this behavior can be changed to the previous one (saving in `projects/projectname/dat` as in QPROP v3.2) by enabling the preprocessor directive `SAVE_NEXT_TO_PROJECT` in `commonvars.hh`.
  - One can also run new variations of the same project (called `experiments`) by adding `exp-name string newvariation` to the projects `config.param` file. This will place new files in `~/Results/projectname/newvariation` or `projects/projectname/dat/newvariation` respectively.
- More flexibility in output file naming. Apart from the standard naming mentioned above, the user can easily add his own string to the output files using `im_extraid` or `re_extraid`. As the name suggests `im_extraid` changes names of files output by `im` routine. In the output of `re` and later routines names are created using `re_extraid + im_extraid` since those depend on the `im` routine output. See: `configCustomVars()` function in `template` project.
- The contents of the `-observ.dat` files are now dependent on preprocessor directives (flags) located on top of the project's `config.hh` file. This allows for less checks in runtime, improves visibility (compared with .param files). Commenting a flag with `CTRL + /` or `CMD+ /` disables a subroutine. Recompilation is of course required, but it is a minor inconvenience, since for most projects we will already know what we want to compute beforehand.
  - Apart from the quantities relevant to the `-observ.dat` file, also the `tsurff-method` parameter has been ditched for preprocessor directives (more below).
- Preprocessor directives for iSURFV/tSURFF. Since v3.2 iSUFRV can be considered the default method of computing electron spectra in QPROP with tSURFF being auxiliary. Heuristic tSURFF spectrum is computed anyway (as it was in v3.2) while computing iSURFV. To compute momentum spectra one needs to activate (define) the `MOMENTUM_DATA` flag in project's `config.hh` file. To activate full, old tSURFF routine one needs to **additionally** activate the flag `EXTRA_TIME_TSURFF_ONLY`.
- Vector potentials, together with scalar and imaginary potentials have been given their own classes from which derived classes are built. This allows not only for building a repository of constantly used potential types in `potentials.hh`, but also aggregates options and derived variables (such as Up or quiver amplitude) for those potentials. This is particularly useful when building log files (which still need some improvements). Setting vector potentials is now done in exposed `config.hh` function `configPulse()` and has a very simple form. Example:

  ```
    vecpot_x = new Zero_vecpot();
    vecpot_y = new Zero_vecpot();
    vecpot_z = new SinEnvSin2_vecpot(omega1, n1z, mult * E1z, phase1z);
  ```

  - Notice how C++ variables correspond to their `.param` counterparts, only exchanging `-` for `_`.

- Custom vector potentials (scalar and imaginary too) can be defined inside `config.hh` files by inheriting from `vecpot`, `scalarpot` and `imaginarypot` base classes. See the commented example in `template/config.hh`.
- Static and imaginary potentials are to be set in `configPotentials()` function in `config.hh`. `pot_cutoff` variable is calculated there as well, if `MOMENTA_DATA` preprocessor define is active. This is an purposeful example illustrating the use of preprocessor directives for conditional code compilation.
- The change in grid size usually occouring due to the tsurff methods have been delegated to the `correctGridSize()` declared and defined in `config.hh`. There's currently a small comment discussion in `template/config.hh` as it's not crystal clear to me what is the "right" way to initialize the `radial_grid_size`. Feedback would be appreciated.
- By default `im` routine initializes from analytic expressions for eigenstates, which makes finding precise eigenstates a little bit faster. This is a new addition.
- Powerful new scripts. `run` and `remote` used for running locally and remotely.

## A few others:

- common-variables-for-make.mk is now shared by main `Makefile` and `base/Makefile`.
- A few projects have been added by me, as I have been trying to unify my dev environment. `neon` and `l-states-cutoff` are not yet finished. `rydberg-pop` project might be useful as it uses new routines to calculate the populations projecting the state to some precomputed eigenstates. It is a simple, but new addition which needs a little more documentation.
- There's a trashbin in which I try to recycle scripts - called `wip` for "work in progress"
