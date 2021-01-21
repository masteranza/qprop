// #define HHG_DATA          //Whether to export high harmonics data
// #define ENERGY_DATA       //Whether to save energy data
// #define NORM_DATA         //Whether to save norm data
// #define ZEXP_DATA         //whether to save z_expect data
// #define PROJ_DATA         //Whether to export projections
// #define PROJ_DATA_CONST_L //If enabled only states with the same L will be projected
// #define PROJ_DATA_ABS     //Write projections in abs format instead of real&imag
#define MOMENTA_DATA
// #define EXTRA_TIME_TSURFF_ONLY

void configPotentials()
{
#ifdef MOMENTA_DATA
  //To ensure that even highly excited bound states are negligable for r>R_tsurff.
  //Also in compliance with the rule 2*R_co< R_tsurff
  pot_cutoff = R_tsurff / 4.0;
  scalarpotx = new CutCoulomb_scalarpot(nuclear_charge, pot_cutoff);
#else
  pot_cutoff = 0.0;
  scalarpotx = new scalarpot(nuclear_charge);
#endif
#ifdef IM //In imaginary time propagation we (usually) don't want absorbing boundary conditions
  imaginarypot = new imagpot(0, 0.0);
#else               //Shared by ./re; ./isurfv, ./tsurff
  imaginarypot = new imagpot(imag_width_ngps);
#ifdef MOMENTA_DATA //Extending imaginary potential width for isurfv routines
  extended_imaginarypot = new imagpot(extended_imag_width_ngps, imag_ampl);
#endif
#endif
}

void configPulse()
{
  //initialize vector potentials
  vecpot_x = new Zero_vecpot();
  vecpot_y = new Zero_vecpot();
  vecpot_z = new SinEnvSin2_vecpot(omega1, n1z, mult * E1z, phase1z);

  pulse_duration = (qprop_dim == 34) ? vecpot_z->get_duration() : max(vecpot_x->get_duration(), vecpot_y->get_duration());
  duration = pulse_duration + additional_time;

  U_p = (qprop_dim == 34) ? vecpot_z->get_Up() : max(vecpot_x->get_Up(), vecpot_y->get_Up());

  quiver_amplitude = (qprop_dim == 34) ? vecpot_z->get_quiver_amplitude() : max(vecpot_x->get_quiver_amplitude(), vecpot_y->get_quiver_amplitude());
#ifdef EXTRA_TIME_TSURFF_ONLY
  duration += time_surff;
#endif
}
void processCustomOptions(int argc, char *argv[])
{
}
void configCustomVars()
{
}
void correctGridSize()
{
  radial_grid_size = re_radial_grid_size;
#ifdef MOMENTA_DATA
  radial_grid_size = imag_width + R_tsurff + quiver_amplitude;
  extended_radial_grid_size = extended_imag_width + R_tsurff + quiver_amplitude;
#endif
}