#define TS
#include <iostream>
#include <memory>
#include <complex>
typedef std::complex<double> cplxd;
typedef std::unique_ptr<cplxd[]> cplxd_ptr;
#include <grid.h>
#include <fluid.h>
#include <hamop.h>
#include <wavefunction.h>
// #include <parameter.hh>
// #include <smallHelpers.hh>
#include <powers.hh>
#include "potentials.hh"
#include <commonvars.hh>
#include "config.hh"
#include <getopt.h>
#include <tsurffSpectrum.hh>

using std::cout;
using std::string;
void mergeOutputFiles(string sfile, int num_proc)
{
    ifstream inpf;
    ofstream outf;
    char ch;

    outf.open(dir_name + sfile + ".dat");
    for (int ii = 0; ii < num_proc; ii++)
    {
        inpf.open(dir_name + sfile + fixed_length_number(num_proc, ii) + ".dat");
        while (inpf >> noskipws >> ch)
        {
            outf << ch;
        }
        inpf.close();
        if (remove((dir_name + sfile + fixed_length_number(num_proc, ii) + ".dat").c_str()) != 0)
            cout << "Could not remove " << sfile << fixed_length_number(num_proc, ii) << ".dat file";
    }
    outf.close();
}

int main(int argc, char **argv)
{
#ifndef MOMENTA_DATA
    cout << "Error: MOMENTA_DATA preprocessor directive not active" << endl;
    exit(-1);
#endif
    int num_proc = 1;
#ifdef HAVE_MPI
    int ierr = MPI_Init(&argc, &argv);
    cout << "MPI_Init returns " << ierr << endl;
    int i_proc;
    MPI_Comm_rank(MPI_COMM_WORLD, &i_proc);
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    if (i_proc == 0)
#endif
#ifndef EXTRA_TIME_TSURFF_ONLY
        print_banner("I-SURFV + heuristic TSURFF initialized");
#else
    print_banner("T-SURFF only initialized");
#endif
    clock_t t = clock();
    int ind = processOptions(argc, argv);
    //OVERRIDE VARS WITH CUSTOM INIT (defined in config.hh)
    //Passing only unprocessed options
    processCustomOptions(argc - ind, &argv[ind]);
    configVars();
    configCustomVars();
    if (ts_log_interval)
    {
#ifdef HAVE_MPI
        if (i_proc == 0)
            file_log = fopen_with_check(dir_name + ts_fname_log, "w", verbose);
        else
            file_log = fopen_with_check(dir_name + ts_fname_log, "a", verbose);
#endif
    }
    configPulse();
    correctGridSize();
    configPotentials();
#ifdef HAVE_MPI
    if (i_proc == 0)
#endif
        logConfig();

    tsurffSpectrum<vecpot, vecpot, vecpot> tsurff(std::ref(*vecpot_x), std::ref(*vecpot_y), std::ref(*vecpot_z));
    tsurff.time_integration();
    // tsurff.print_int_dt_psi();
    // tsurff.print_wigner(0);
    // tsurff.print_Plms();
    // tsurff.print_bessel();
    tsurff.polar_spectrum();
    if (expansion_scheme == 2)
        tsurff.partial_amplitudes();

#ifdef HAVE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    if (i_proc == 0)
        logAdd(" ---------------------------------------------------\n");
    sleep(0.1);
    int ierrfin = MPI_Finalize();
    logAdd("Finished on proc %d. MPI_Finalize returns %d \n", i_proc, ierrfin);
    sleep(0.1); //To avoid printing the above after what's below
    if (i_proc == 0)
#endif
    {
        logAdd(" ---------------------------------------------------\n");
        logAdd("Merging 'polar' files...\n");
        mergeOutputFiles(ts_fname + "-polar", num_proc);
#ifndef EXTRA_TIME_TSURFF_ONLY
        mergeOutputFiles(is_fname + "-polar", num_proc);
#endif
        if (expansion_scheme == 2)
        {
            logAdd("Merging 'partial' files...\n");
            mergeOutputFiles(ts_fname + "-partial", num_proc);
#ifndef EXTRA_TIME_TSURFF_ONLY
            mergeOutputFiles(is_fname + "-partial", num_proc);
#endif
        };
        // }
        float sec = ((float)(clock() - t)) / CLOCKS_PER_SEC;
        logAdd("Execution time: %4dh %3dmin %3dsec\n", long(sec) / 3600, long(sec) % 3600 / 60, (long(sec * 100) % 6000) / 100);
    }
#ifdef HAVE_MPI
    stop_log(i_proc);
#endif
    return 0;
}