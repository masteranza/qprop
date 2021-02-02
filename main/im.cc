#define IM
#include <iostream>
#include <complex>
#include <functional>
#include <string>
#include <time.h>

#include <bar.h>
#include <fluid.h>
#include <grid.h>
#include <hamop.h>
#include <wavefunction.h>
#include <powers.hh>
// #include <parameter.hh>
// #include <smallHelpers.hh>

#include "potentials.hh"
#include <commonvars.hh>
#include "config.hh"

/// -----------------------------------------------------------------------------------------------
/// This code works by generating stationary states of a given l (l-qnumber),
/// up to state n (max_n_number)
/// You can optionally specify the max n state and target l state using the first and second argument
/// and the nuclear_charge using the third argument
/// -----------------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    clock_t t = clock();
    print_banner("im: Imaginary time propagation");
    int ind = processOptions(argc, argv);
    //OVERRIDE VARS WITH CUSTOM INIT (defined in config.hh)
    //Passing only unprocessed options
    processCustomOptions(argc - ind, &argv[ind]);
    configVars();
    configCustomVars();
    if (im_log_interval)
        file_log = fopen_with_check(dir_name + im_fname_log, "w", verbose);
    if (potential_log)
        file_potential = fopen_with_check(dir_name + im_fname_potential, "w", verbose);

    double acc, E_tot_prev, E_tot;
    const cplxd timestep(0.0, -1.0 * imag_timestep);
    grid g;
    hamop hamilton;
    wavefunction staticpot;

    //GRID SETUP
    g.set_dim(qprop_dim);
    g.set_ngps(long(im_radial_grid_size / delta_r), l_qnumber + 1, 1);
    g.set_delt(delta_r);
    g.set_offs(0, 0, 0);

    //STATE INITS
    wavefunction wf[max_n_number - l_qnumber]; //there are exactly n_max-l states to calculate, because lower n states don't have some l's.
    const int me = 0;                          // Dummy here
    for (long nr1 = 0; nr1 < max_n_number - l_qnumber; nr1++)
    {
        wf[nr1].init(g.size());
        if (qprop_dim == 34)
        {
            wf[nr1].init(g, nr1 + l_qnumber + 1, l_qnumber, nuclear_charge);
        }
        else
        {
            wf[nr1].init_rlm(g, nr1 + l_qnumber + 1, l_qnumber, m_qnumber, nuclear_charge);
        }
        wf[nr1].normalize(g);
    };

    //HAMILTONIAN
    configPotentials();
    logConfig();
    staticpot.init(g.size());
    hamilton.init(g, always_zero2, always_zero2, always_zero2, std::ref(*scalarpotx), std::ref(*imaginarypot));
    staticpot.calculate_staticpot(g, hamilton);

    logPotential(hamilton);
    logSilent(g);

    //IMAGINARY TIME PROPAGATION
    for (long nr1 = 0; nr1 < max_n_number - l_qnumber; nr1++)
    {
        logAdd("   CALCULATING STATE Z=%.5e n=%d, l=%d, m=%d\n", nuclear_charge, (int)nr1 + 1 + l_qnumber, l_qnumber, m_qnumber);
        logAdd("   Initial wf norm: %.10e\n", wf[0].norm(g));
#ifdef IM_TIMESTEP_BOOST
        //When calculating higher excited states it (sometimes) appears to be usefull to increase the timestep
        double boost = (nr1 * 1.0 + 1.0);
        logAdd("   Current imag timestep %.10e\n", boost * imag(timestep));
#else
        double boost = 1.0;
#endif
        logAdd("%7s/%7s%25s%25s\n", "STEP", "TOTAL", "ENERGY", "ACCURACY");

        E_tot = 0.0;
        acc = 1.0;
        long ts = 0;

        // TODO: add m support!
        while (acc > target_accuracy && ts < max_steps)
        {
            for (long nr2 = 0; nr2 < nr1; nr2++)
            {
                wf[nr1].subtract(g, wf[nr2]);
            };
            wf[nr1].propagate(boost * timestep, 0.0, g, hamilton, me, staticpot, m_qnumber, nuclear_charge);
            wf[nr1].extract_ell_m(g, l_qnumber, m_qnumber);
            wf[nr1].normalize(g);

            const double time = double(ts) * boost * imag(timestep);
            E_tot_prev = E_tot;
            E_tot = real(wf[nr1].energy(0.0, g, hamilton, me, staticpot, nuclear_charge));
            acc = fabs((E_tot_prev - E_tot) / (E_tot_prev + E_tot));

            if (im_log_interval && (ts % im_log_interval == 0))
            {
                logAdd("%7ld/%7ld%25.15e%25.15e\n", ts, max_steps, E_tot, acc);
            }
            ts++;
        };
        if (im_log_interval && ((ts - 1) % im_log_interval != 0)) //Only print this if last wasn't printed
            logAdd("%7ld/%7ld%25.15e%25.15e\n", ts, max_steps, E_tot, acc);
        // logAdd("\nn=%d, l=%d, m=%d, Z=%d, E,dE=%20.15le,%20.15le\n", (int)nr1 + l_qnumber + 1, l_qnumber, m_qnumber, (int)nuclear_charge, E_tot, acc);
        logAdd("Final wf norm: %le\n", wf[nr1].norm(g));

        FILE *file_wf_fin = fopen_with_check(dir_name + im_fname_wfs[nr1], "w", verbose);
        wf[nr1].dump_to_file_sh(g, file_wf_fin, 1, verbose);
        fclose(file_wf_fin);
    };

    float sec = ((float)(clock() - t)) / CLOCKS_PER_SEC;
    logAdd("Execution time: %4dh %3dmin %3dsec\n", long(sec) / 3600, long(sec) % 3600 / 60, (long(sec * 100) % 6000) / 100);

    stop_log();
    return 0;
}
