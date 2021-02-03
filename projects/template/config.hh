//Observer actions at every obser_log_interval step of real time propagation
#define HHG_DATA          //Whether to export high harmonics data
#define ENERGY_DATA       //Whether to save energy data
#define NORM_DATA         //Whether to save norm data
#define ZEXP_DATA         //whether to save z_expect data
#define PROJ_DATA         //Whether to export projections
#define PROJ_DATA_CONST_L //If enabled only states with the same L will be projected
#define PROJ_DATA_ABS     //Write projections in abs format instead of real&imag
#define MOMENTA_DATA      //Whether to save data for isurfv/tsurff
// #define EXTRA_TIME_TSURFF_ONLY //Whether to use old tsurff approach with long propagation time

// Warning: getopt_long used in commonvars.hh may reorder your arguments
// to stop this insert them after ' -- ' you can also use names but probably not getopt again.
// Example: calling ./im with charge Z=1 and custom arguments 3.1 and 4.1 to be extracted in processCustomOptions
// im -Z 1 -- 3.1 4.1
// If you pass your args like this
// ./im 3.1 4.1 -Z 1 they will get reordered to ./im -Z 1 3.1 4.1
// which looks ok, but the correct order might not be guaranteed in the custom args or it may cause errors
// The function processCustomOptions will get only your custom options, which means 3.1 4.1 (in the example above)
void processCustomOptions(int argc, char *argv[])
{
  for (int i = 0; i < argc; i++)
  {
    // cout << argv[i] << endl;
  }
}

// This function can be used to hard-code variables for all the routines.
// It gets run last, after the .params files and command line options are parsed.
void configCustomVars()
{
  //  You can reformat filenames to:
  // 'im-im_extraid-nX-lX-mX...' for IM files
  // 'prefix-re_extraid-im_extraid-nX-lX-mX...' for RE files
  //  by changing im_extraid and re_extraid.
  //  Remember to call initFilenames() afterwards.
  im_extraid = str_nuclear_charge();
  re_extraid = "-MY";
  initFilenames();
}

// This function should be used to set scalar and imaginary potentials.
// One can use preprocessor defines to set them differently depending on routine.
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

// Here you set your vector potentials.
// Some predefined classes can be found in main/potentials.hh
// If you need something else entirely uncomment the code located at the bottom of this file, customize and move it to the top of the file
// You should also decide on your own how to calculate pulse related variables, like quiver.
void configPulse()
{
  //Look into potentials.hh to find predefined vecpot-derived classes.
  // vecpot_x = Two_vectpot(omega1, omega2, n1x, n2x, delay1, delay2, E_mult * E1x, E_mult * E2x, phase1x, phase2x);
  // vecpot_y = Two_vectpot(omega1, omega2, n1y, n2y, delay1, delay2, E_mult * E1y, E_mult * E2y, phase1y, phase2y);
  vecpot_x = new Zero_vecpot();
  vecpot_y = new Zero_vecpot();
  vecpot_z = new SinEnvSin2_vecpot(omega1, n1z, delay1, E_mult * E1z, phase1z);

  pulse_duration = (qprop_dim == 34) ? vecpot_z->get_duration() : max(vecpot_x->get_duration(), vecpot_y->get_duration());
  duration = pulse_duration + additional_time;

  U_p = (qprop_dim == 34) ? vecpot_z->get_Up() : max(vecpot_x->get_Up(), vecpot_y->get_Up());

  quiver_amplitude = (qprop_dim == 34) ? vecpot_z->get_quiver_amplitude() : max(vecpot_x->get_quiver_amplitude(), vecpot_y->get_quiver_amplitude());
#ifdef EXTRA_TIME_TSURFF_ONLY
  duration += time_surff;
#endif
}
// This is used to set the new size of the grid in radial_grid_size
// And the extended_grid_size for the isurfv routine
void correctGridSize()
{
  radial_grid_size = re_radial_grid_size;
#ifdef MOMENTA_DATA
  // OPTION 1:
  // According to the papers it sufices that R_tsurff is larger than the classical quiver amplitude.
  // So to be safe we make sure it's at least two times as big.
  // Also, correct me if I'm wrong, but the previous method, i.e., grid_size= imag_width+R_tsurff+quiver_amplitude,
  // would, in fact just produce a large and unnecessary(?) free space between tsurff surface and the imagininary potential
  // if (R_tsurff < 2.0 * quiver_amplitude)
  // log_add("R_tsurff is smaller than two quiver amplitudes and will be automatically increased.");
  // R_tsurff = max(R_tsurff, 2.0 * quiver_amplitude);

  // OPTION 2:
  radial_grid_size = imag_width + R_tsurff + quiver_amplitude;
  extended_radial_grid_size = extended_imag_width + R_tsurff + quiver_amplitude;

  // OPTION 3:
  // if (radial_grid_size < re_radial_grid_size)
  // {
  //   printf("TSurff grid size ignored, as radial-grid-size (from propagate.param) appears larger");
  //   radial_grid_size = re_radial_grid_size;
  // }
  // else
  //   printf("Using TSurff grid size (imag_width + R_tsurff + quiver_amplitude)=%le [au]\n", radial_grid_size);

  // "DEBUG MODE" OPTION:
  // radial_grid_size = im_radial_grid_size;

#endif
}

//Here you can define your own custom potentials (vector,scalar and imaginary)
//see potentials.hh to get the idea what methods need to be defined.
// class customVecPot : public vecpot
// {
// };

// class customScalarpot : public scalarpot
// {

// };

// class customImagpot : public imagpot
// {

// };
