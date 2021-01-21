#define IS
#include <iostream>
#include <vector>
#include <complex>
#include <time.h>
typedef std::complex<double> cplxd;

#include <wavefunction.h>
#include <fluid.h>
#include <grid.h>
#include <hamop.h>
#include <gfunc.h>
// #include <smallHelpers.hh>
//#include <gsl/gsl_sf_legendre.h>
// #include <parameter.hh>
#include <powers.hh>
// Definitions of potentials
#include "potentials.hh"
#include <commonvars.hh>
#include "config.hh"

using std::cout;
using std::endl;
using std::vector;

int main(int argc, char **argv)
{
#ifndef MOMENTA_DATA
    cout << "Error: MOMENTA_DATA preprocessor directive not active" << endl;
    exit(-1);
#endif
    clock_t t = clock();
    print_banner("isurfv: Applying the Green's function 1/(H-Ek) to Psi and dPsi/dr");
    int ind = processOptions(argc, argv);
    //OVERRIDE VARS WITH CUSTOM INIT (defined in config.hh)
    //Passing only unprocessed options
    processCustomOptions(argc - ind, &argv[ind]);
    configVars();
    configCustomVars();
    if (is_log_interval)
        file_log = fopen_with_check(dir_name + is_fname_log, "w", verbose);
    configPulse();
    correctGridSize();
    configPotentials();
    logConfig();

    // Set verbosity
    const int iv = 1; // 1 leads to common output, 0 is silent regime

    grid g; //Extended grid due to larger imag_width
    g.set_dim(qprop_dim);
    g.set_ngps(long(extended_radial_grid_size / delta_r), re_l_grid_size, 1);
    g.set_delt(delta_r);
    g.set_offs(0, 0, 0);

    grid g_small; //Standard sized grid as in ./re
    g_small.set_dim(qprop_dim);
    g_small.set_ngps(long(radial_grid_size / delta_r), re_l_grid_size, 1);
    g_small.set_delt(delta_r);
    g_small.set_offs(0, 0, 0);

    wavefunction psi_at_RI, d_psi_dr_at_RI;
    if (qprop_dim == 34)
    {
        psi_at_RI.init(g.ngps_y());
        d_psi_dr_at_RI.init(g.ngps_y());
    }
    else
    {
        psi_at_RI.init(g.ngps_y() * g.ngps_y());
        d_psi_dr_at_RI.init(g.ngps_y() * g.ngps_y());
    }
    fluid V_ee_0;
    V_ee_0.init(g.ngps_x());

    // The Hamiltonian
    hamop hamilton;
    hamilton.init(g, always_zero2, always_zero2, always_zero2, std::ref(*scalarpotx), std::ref(*extended_imaginarypot));

    FILE *file_wf = fopen_with_check(dir_name + re_fname_wffinal, "r", verbose);
    // The wavefunction arrays
    wavefunction wf, wf_load;
    wf.init(g.size());
    wf_load.init(g_small.size());
    wf_load.init(g_small, file_wf, 0, 1);
    fclose(file_wf);
    wf.regrid(g, g_small, wf_load);
    // Display the grid parameters
    logSilent(g);

    wavefunction staticpot;
    staticpot.init(g.size());
    staticpot.calculate_staticpot(g, hamilton);

    // Output files
    ofstream file_res((dir_name + is_fname + "-isurfv.dat"));
    file_res.precision(17);

    // Energy loop
    for (long i_energy = 0; i_energy < num_k_surff; i_energy++)
    {
        if (is_log_interval && (i_energy % is_log_interval == 0))
        {
            logAdd("%ld energies of %ld complete\n", i_energy, num_k_surff);
        };

        double energy, delta_energy;
        if (delta_k_scheme == 1)
        {
            delta_energy = energy_max / (double(num_k_surff) * double(num_k_surff));
            energy = double(i_energy) * double(i_energy) * delta_energy;
        }
        else if (delta_k_scheme == 2)
        {
            delta_energy = energy_max / double(num_k_surff);
            energy = i_energy * delta_energy;
        };
        gfunc(psi_at_RI, d_psi_dr_at_RI, energy, staticpot, V_ee_0, nuclear_charge, g, wf, R_tsurff);
        // write the partial results (i.e., for individual l (and m)) to file
        file_res << energy << " " << sqrt(2.0 * energy) << " ";

        for (long l_index = 0; l_index < g.ngps_y(); l_index++)
        {
            if (qprop_dim == 34) // Linear polarization and fixed value of m
            {
                file_res << real(psi_at_RI[l_index]) << " " << imag(psi_at_RI[l_index]) << " " << real(d_psi_dr_at_RI[l_index]) << " " << imag(d_psi_dr_at_RI[l_index]) << " ";
            }
            else // Polarization in the XY plane
            {
                const long m_limit = l_index;
                for (long m_index = -m_limit; m_index <= m_limit; m_index++)
                {
                    long lmindex = l_index * (l_index + 1) + m_index;
                    file_res << real(psi_at_RI[lmindex]) << " " << imag(psi_at_RI[lmindex]) << " " << real(d_psi_dr_at_RI[lmindex]) << " " << imag(d_psi_dr_at_RI[lmindex]) << " ";
                };
            }
        };
        file_res << endl; // Break line for every new energy
    };                    // End of energy loop
    float sec = ((float)(clock() - t)) / CLOCKS_PER_SEC;
    logAdd("Execution time: %4dh %3dmin %3dsec\n", long(sec) / 3600, long(sec) % 3600 / 60, (long(sec * 100) % 6000) / 100);
    stop_log();
    return 0;
}