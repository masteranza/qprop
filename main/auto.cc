#define AUTO
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
/// This file is a placeholder for more general auto-optimization of settings based on
/// meta parameters like target-energy (for now only this)
/// -----------------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    clock_t t = clock();
    print_banner("auto: Finding nunclear_charge to given (ground state) target-energy");
    int ind = processOptions(argc, argv);
    //OVERRIDE VARS WITH CUSTOM INIT (defined in config.hh)
    //Passing only unprocessed options
    processCustomOptions(argc - ind, &argv[ind]);
    configVars();
    configCustomVars();

    if (target_energy == 0.0)
    {
        logAdd("    Nothing to do, target-energy not set - exiting.\n");
    }
    else if (target_energy > 0.0)
    {
        logAdd("   Error: target-energy should be negative!\n");
        exit(-1);
    }
    else
    {
        if (im_log_interval)
            file_log = fopen_with_check(dir_name + auto_fname_log, "w", verbose);

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
        wavefunction wf[1]; //there are exactly n_max-l states to calculate, because lower n states don't have some l's.
        const int me = 0;   // Dummy here
        for (long nr1 = 0; nr1 < 1; nr1++)
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
        staticpot.init(g.size());
        hamilton.init(g, always_zero2, always_zero2, always_zero2, std::ref(*scalarpotx), std::ref(*imaginarypot));
        staticpot.calculate_staticpot(g, hamilton);

        int max_tries = 1000;
        logAdd("   Looking for appropriate nuclear_charge given the target_energy=%e\n", target_energy);
        int prec = getRequiredPrecision(target_energy);
        logAdd("   The error should be smaller than: %e\n", pow(10.0, -prec));

        while (abs(E_tot - target_energy) > pow(10.0, -prec) && max_tries--)
        {
            E_tot = 0.0;
            acc = 1.0;
            long ts = 0;
            //We need to reinit those each time
            configPotentials();
            hamilton.init(g, always_zero2, always_zero2, always_zero2, std::ref(*scalarpotx), std::ref(*imaginarypot));
            staticpot.calculate_staticpot(g, hamilton);
            logAdd("   Current nuclear-charge=%e, diff=%e, tries left=%d\n", nuclear_charge, abs(E_tot - target_energy), max_tries);
            logAdd("%7s/%7s%25s%25s\n", "STEP", "TOTAL", "ENERGY", "ACCURACY");

            while (acc > target_accuracy && ts < max_steps)
            {
                wf[0].propagate(timestep, 0.0, g, hamilton, me, staticpot, m_qnumber, nuclear_charge);
                wf[0].extract_ell_m(g, l_qnumber, m_qnumber);
                wf[0].normalize(g);

                const double time = double(ts) * imag(timestep);
                E_tot_prev = E_tot;
                E_tot = real(wf[0].energy(0.0, g, hamilton, me, staticpot, nuclear_charge));
                acc = fabs((E_tot_prev - E_tot) / (E_tot_prev + E_tot));

                if (im_log_interval && (ts % im_log_interval == 0))
                {
                    logAdd("%7ld/%7ld%25.15e%25.15e\n", ts, max_steps, E_tot, acc);
                }
                ts++;
            };
            logAdd("%7ld/%7ld%25.15e%25.15e\n", ts, max_steps, E_tot, acc);

            //Only update nuclear_charge if another run is expected
            if (abs(E_tot - target_energy) > pow(10.0, -prec) && max_tries)
            {
                // Not going straight with the analytical formula as on grid we could overshoot
                // Taking 2/3 of the step instead
                nuclear_charge *= (2.0 * sqrt(abs(target_energy / E_tot)) + 1.0) / 3.0;
            }
        };
        logAdd("   Found best nuclear-charge=%e yielding energy=%e (target-energy=%e)\n\n", nuclear_charge, E_tot, target_energy);
        float sec = ((float)(clock() - t)) / CLOCKS_PER_SEC;
        logAdd("Execution time: %4dh %3dmin %3dsec\n", long(sec) / 3600, long(sec) % 3600 / 60, (long(sec * 100) % 6000) / 100);
        stop_log();

        char dest_string[20];
        sprintf(dest_string, "%.15e", nuclear_charge);
        //Update the config.param file
        para->updateParamFileField("nuclear-charge", to_string(dest_string));
        //Save a copy of the merged param file
        para->copyMergedParamFileTo(dir_name + auto_fname + "-" + conf_file + confext);
    }
    return 0;
}
