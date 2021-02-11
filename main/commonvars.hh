// If you really want to save the results in the project folder
// Enable the following flag
// #define SAVE_NEXT_TO_PROJECT
// By default the filename strings contain extra 0 pading for n,l,m numbers (for two digit numbers).
// If you don't like that, set the following flag to "" (empty string)
#define STATE_NAME_PADDING ".2"
// Used to increase the accuracy of target-energy found automatically in IM
#define IM_EXTRA_PRECISION 1
// If you want n,l,m state info in re filenames enable this
// #define INCLUDE_STATE_INFO_IN_RE
// TODO: Add optional colored output (someday)
// #define RESET   "\033[0m"
// #define BLACK   "\033[30m"      /* Black */
// #define RED     "\033[31m"      /* Red */
// #define GREEN   "\033[32m"      /* Green */
// #define YELLOW  "\033[33m"      /* Yellow */
// #define BLUE    "\033[34m"      /* Blue */
// #define MAGENTA "\033[35m"      /* Magenta */
// #define CYAN    "\033[36m"      /* Cyan */
// #define WHITE   "\033[37m"      /* White */
// #define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
// #define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
// #define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
// #define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
// #define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
// #define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
// #define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
// #define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#include <smallHelpers.hh>
#include <parameter.hh>
#include <ctime>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
//Femtosecond is this many atomic units
const double fs = 41.34138;
//Unit intensity
const double unit_intensity = 3.50944e16;
//How much information to show during run 0, 1 or higher
int verbose = 0;
//Whether to continue re from previous final wf instead of output of im routine 
bool re_continue = 0;
//GLOBALS FOR THE PROJECTS READ FROM THE .PARAM FILE(S)

//EXPERIMENT META
//Orginally passed options
string orgArgv = "";
std::vector<std::string> cmdLineNotes;
//Path to the project config file and path to (fallback) defualts confing file
const string confext(".param");
string this_dir("./");
string conf_file("config");
string defu_file("../../main/default-config");
//Pointer to the parameter list
parameterList *para;
//Common file pointers, NULL is necessary to tell whether we wan't log to happen or not.
FILE *file_log = NULL;
FILE *file_observer = NULL;
FILE *file_potential = NULL;
FILE *file_vpotential = NULL;
//Experiment name (subfolder in project)
string exp_name;
//File names and their parts
string str_reim_fname = "";
string im_extraid = "";
string re_extraid = "";
string im_str_common, str_max_n_qnumber, str_l_qnumber, str_m_qnumber, str_initial_n, str_initial_l, str_initial_m;
string im_fname_wfs[1000];
string auto_fname_log, im_fname_log, re_fname_log, is_fname_log, ts_fname_log;
string im_fname_potential, re_fname_observer, re_fname_wf, re_fname_vpotential;
string re_fname_wfsnap, re_fname_wffinal, auto_fname, re_fname, ts_fname, is_fname;
string is_fname_isurfv, ts_fname_psi, ts_fname_dpsidr;
//Prefixes for different routines
string auto_prefix("/auto");
string re_prefix("/re");
string im_prefix("/im");
string isurfv_prefix("/is");
string tsurff_prefix("/ts");
//Init the default directories
#ifdef SAVE_NEXT_TO_PROJECT
string dir_name = create_dir_here("/dat");
#else
string dir_name = create_dir_in_home("/Results/" + to_string(STRINGIFY(PROJNAME)));               //Note the slash at the beginning!
#endif
//Vars
long im_log_interval, re_log_interval, is_log_interval, ts_log_interval, wf_log_interval, obser_log_interval, max_steps;
bool potential_log, vpotential_log, finalwf_log;

//ATOM
double target_accuracy, nuclear_charge, target_energy;
long qprop_dim, max_n_number, l_qnumber, m_qnumber, re_initial_n, re_initial_l, re_initial_m;
scalarpot *scalarpotx;
imagpot *imaginarypot;
imagpot *extended_imaginarypot;
vector<double> super_weights;
vector<long> super_positions;
bool re_superpositions = false;
//SPACETIME
double delta_r, dt, imag_timestep, im_radial_grid_size, re_radial_grid_size, imag_width, imag_ampl;
double radial_grid_size; //This one will be used in the end in re/tsurff
long re_l_grid_size, imag_width_ngps;

//LASER
double E1x, E2x, E1y, E2y, E1z, E2z, n1x, n2x, n1y, n2y, n1z, n2z, delay1, delay2, omega1, omega2;
double phase1x, phase1y, phase1z, phase2x, phase2y, phase2z, intensity_mult, E_mult, pulse_duration, duration;
//Other derived parameters
double quiver_amplitude, gamma_K, U_p, I_p;
vecpot *vecpot_x;
vecpot *vecpot_y;
vecpot *vecpot_z;
//TSURFF
double additional_time, R_tsurff, time_surff, pot_cutoff, k_max_surff, w_imag_fact, extended_imag_width, energy_max, extended_radial_grid_size;
long expansion_scheme, num_k_surff, num_theta_surff, num_phi_surff, delta_k_scheme, cache_size_t, extended_imag_width_ngps;

void logAdd(char const *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  if (file_log != NULL)
  {
    va_start(ap, fmt);
    vfprintf(file_log, fmt, ap);
    va_end(ap);
    fflush(file_log);
  }
}

//TODO: INIT only what's really needed.
void loadParams(string conf_file, string defu_file)
{

  para = new parameterList(this_dir + conf_file + confext, this_dir + defu_file + confext);
  //EXPERIMENT META
  exp_name = para->getString("exp-name", true);
  im_log_interval = para->getLong("im-log-interval");
  re_log_interval = para->getLong("re-log-interval");
  is_log_interval = para->getLong("is-log-interval");
  ts_log_interval = para->getLong("ts-log-interval");
  potential_log = para->getBool("potential-log");
  wf_log_interval = para->getLong("wf-log-interval");
  finalwf_log = para->getBool("finalwf-log");
  vpotential_log = para->getBool("vpotential-log");
  obser_log_interval = para->getLong("obser-log-interval");
  max_steps = para->getLong("max-steps");
  target_accuracy = para->getDouble("target-accuracy");

  //ATOM
  qprop_dim = para->getLong("qprop-dim"); //Geometry of the problem
  nuclear_charge = para->getDouble("nuclear-charge");
  max_n_number = para->getLong("max-n-qnumber");
  l_qnumber = para->getLong("l-qnumber");
#ifdef AUTO
  logAdd("   Warning: Resetting max_n_number=1, l_qnumber=0 for ./auto - make sure this is intended.\n");
  max_n_number = 1;
  l_qnumber = 0;
#endif
  m_qnumber = para->getLong("m-qnumber");

  re_initial_n = para->getLong("initial-n");
  re_initial_l = para->getLong("initial-l");
  re_initial_m = para->getLong("initial-m");
  target_energy = para->getDouble("target-energy", true);
  super_positions = para->getVectorLong("super-positions", true);
  re_superpositions = !super_positions.empty();
  super_weights = para->getVectorDouble("super-weights", super_positions.empty());

  if (re_superpositions)
  {
    if (super_positions.size() % 3)
    {
      logAdd("super_positions.size() (%d) not a multiple of 3. super-positions must come in triplets of n l m space separated by space\n", super_positions.size());
      exit(-1);
    }
    if (3 * super_weights.size() != super_positions.size())
    {
      logAdd("super_positions.size() != 3*super_weights.size(), (%d != %d) each superposition has to have a weight\n", super_weights.size(), super_positions.size());
      exit(-1);
    }
  }

  //SPACETIME
  delta_r = para->getDouble("delta-r");
  dt = para->getDouble("delta-t");
  im_radial_grid_size = para->getDouble("im-radial-grid-size");
  re_radial_grid_size = para->getDouble("re-radial-grid-size");
  // const long im_l_grid_size = para->getLong("l-grid-size");
  re_l_grid_size = para->getLong("l-grid-size");
  imag_width = para->getDouble("imag-width");
  imag_ampl = para->getDouble("imag-ampl");

  //LASER
  E1x = para->getDouble("E1-x");
  E2x = para->getDouble("E2-x");
  E1y = para->getDouble("E1-y");
  E2y = para->getDouble("E2-y");
  E1z = para->getDouble("E1-z");
  E2z = para->getDouble("E2-z");
  n1x = para->getDouble("num-cycles-1-x");
  n2x = para->getDouble("num-cycles-2-x");
  n1y = para->getDouble("num-cycles-1-y");
  n2y = para->getDouble("num-cycles-2-y");
  n1z = para->getDouble("num-cycles-1-z");
  n2z = para->getDouble("num-cycles-2-z");
  delay1 = para->getDouble("delay-1");
  delay2 = para->getDouble("delay-2");
  omega1 = para->getDouble("omega-1");
  omega2 = para->getDouble("omega-2");
  phase1x = para->getDouble("phase-1-x");
  phase1y = para->getDouble("phase-1-y");
  phase1z = para->getDouble("phase-1-z");
  phase2x = para->getDouble("phase-2-x");
  phase2y = para->getDouble("phase-2-y");
  phase2z = para->getDouble("phase-2-z");
  intensity_mult = para->getDouble("intensity-mult");

  // TSURFF
  additional_time = para->getDouble("additional-time");
  R_tsurff = para->getDouble("R-tsurff");
  expansion_scheme = para->getLong("expansion-scheme");
  k_max_surff = para->getDouble("k-max-surff");
  num_k_surff = para->getLong("num-k-surff");
  num_theta_surff = para->getLong("num-theta-surff"); //Will be modified to odd number if even
  num_phi_surff = para->getLong("num-phi-surff");
  delta_k_scheme = para->getLong("delta-k-scheme");
  cache_size_t = para->getLong("cache-size-t");
  w_imag_fact = para->getDouble("isurfv-imag-width-factor");
}
//The following are called after init
string imMakeFileName(int n, int l, int m)
{
  char dest_string[12];
  sprintf(dest_string, "-n%" STATE_NAME_PADDING "d-l%" STATE_NAME_PADDING "d-m%" STATE_NAME_PADDING "d", n, l, m);
  return im_prefix + im_extraid + string(dest_string);
}
string str_nuclear_charge()
{
  //For compact representation of floating point numbers
  std::stringstream sstream;
  sstream << "-Z" << nuclear_charge;
  return sstream.str();
}
void initCommonStrings()
{
  char dest_string[5];
  sprintf(dest_string, "-n%" STATE_NAME_PADDING "d", max_n_number);
  str_max_n_qnumber = to_string(dest_string);
  sprintf(dest_string, "-l%" STATE_NAME_PADDING "d", l_qnumber);
  str_l_qnumber = to_string(dest_string);
  sprintf(dest_string, "-m%" STATE_NAME_PADDING "d", m_qnumber);
  str_m_qnumber = to_string(dest_string);
  sprintf(dest_string, "-n%" STATE_NAME_PADDING "d", (int)re_initial_n);
  str_initial_n = to_string(dest_string);
  sprintf(dest_string, "-l%" STATE_NAME_PADDING "d", (int)re_initial_l);
  str_initial_l = to_string(dest_string);
  sprintf(dest_string, "-m%" STATE_NAME_PADDING "d", (int)re_initial_m);
  str_initial_m = to_string(dest_string);
}

void initFilenames(std::string initWFname = "")
{
  auto_fname = auto_prefix + im_extraid;
  auto_fname_log = auto_fname + string(".log");

  im_str_common = im_prefix + im_extraid + str_max_n_qnumber + str_l_qnumber + str_m_qnumber;
  im_fname_log = im_str_common + string(".log");

  im_fname_potential = im_str_common + string("-potential.dat");
  for (int nr1 = 0; nr1 < max_n_number - l_qnumber; nr1++)
  {
    im_fname_wfs[nr1] = imMakeFileName(nr1 + l_qnumber + 1, l_qnumber, m_qnumber) + string("-wf.dat");
  }
  
  //Used to build the file name which we will load
  if (initWFname == string(""))
    re_fname_wf = im_prefix + im_extraid + str_initial_n + str_initial_l + str_initial_m + string("-wf.dat");
  else
    re_fname_wf = initWFname;
  
  str_reim_fname = im_extraid;
#ifdef INCLUDE_STATE_INFO_IN_RE
  if (re_superpositions)
    str_reim_fname += "-superpos";
  else  
    str_reim_fname += str_initial_n + str_initial_l + str_initial_m;
  #endif

  re_fname = re_prefix + re_extraid + str_reim_fname;
  is_fname = isurfv_prefix + re_extraid + str_reim_fname;
  ts_fname = tsurff_prefix + re_extraid + str_reim_fname;

  re_fname_log = re_fname + string(".log");
  is_fname_log = is_fname + string(".log");
  ts_fname_log = ts_fname + string(".log");

  re_fname_observer = re_fname + string("-observ.dat");
  re_fname_vpotential = re_fname + string("-vpotential.dat");
  
  re_fname_wfsnap = re_fname;
  re_fname_wffinal = re_fname + string("-wf[final].dat");
  is_fname_isurfv = is_fname + "-isurfv.dat";
  ts_fname_psi = ts_fname +string("-psi.raw");
  ts_fname_dpsidr = ts_fname + string("-dpsidr.raw");
}


void configVars()
{
  if (qprop_dim != 34 && qprop_dim != 44)
  {
    cerr << "qprop-dim=" << qprop_dim << "is incorrect. Allowed values are 34 (for linear polarization) or 44 (for circular polarization)" << endl;
    exit(-1);
  }
  if (re_initial_l < labs(re_initial_m))
  {
    cerr << "|initial-m|" << labs(re_initial_m) << " is greater than initial-l" << re_initial_l << endl;
    exit(-1);
  };
  if (re_l_grid_size < re_initial_l + 1)
  {
    cerr << "Grid size in L" << re_l_grid_size << " too small for the initial-l=" << re_initial_l << endl;
    exit(-1);
  };
  //Init dir
  if (exp_name != "")
  {
#ifdef SAVE_NEXT_TO_PROJECT
    dir_name = create_dir_here("/dat/" + exp_name);
#else
    dir_name = create_dir_in_home("/Results/" + to_string(STRINGIFY(PROJNAME)) + "/" + exp_name); //Note the slash at the beginning!
#endif
  }
  //Imaginary timestep
  imag_timestep = 0.25 * delta_r;
  // Width of the imaginary potential in grid points
  imag_width_ngps = long(imag_width / delta_r);
  //Intensity multipliers recalculated for electric field multiplier
  E_mult = sqrt(intensity_mult);
  //If we do wf snapshots we certainly want the final wf as well
  if (wf_log_interval > 0)
    finalwf_log = true;
#ifdef MOMENTA_DATA
#ifndef EXTRA_TIME_TSURFF_ONLY
  //If need momenta we also need the final wf
  finalwf_log = true;
#endif
#endif
  // How long do the slowest electrons have time to reach the t-SURFF boundary
  time_surff = R_tsurff / para->getDouble("k-min-tsurff");
  //Size of extended imaginary potential for isurfv
  extended_imag_width = w_imag_fact * imag_width;
  //Same but in grid points
  extended_imag_width_ngps = long(extended_imag_width / delta_r); //Used in isurfv
  //Max energy in tsurff
  energy_max = 0.5 * k_max_surff * k_max_surff;
  //str_nuclear_charge etc.
  initCommonStrings();
  //Inits all filenames used in ./im; ./re; ./isurfv; ./tsurff
  initFilenames();
}

int getRequiredPrecision(double number)
{
  std::stringstream sstream;
  sstream << number;
  int i = 0, L;
  string ss = sstream.str();
  L = ss.length();
  while (ss[i] != '.')
    i++;
  return L - i - 1 + IM_EXTRA_PRECISION;
}

void logVecpot(const vecpot *fpx,
               const vecpot *fpy,
               const vecpot *fpz, int lno_of_ts)
{
  for (long ts = 0; ts < lno_of_ts; ts++)
  {
    const double time = dt * double(ts);
    fprintf(file_vpotential, "%15.10le %15.10le %15.10le %15.10le\n", time, fpx->operator()(time, 0), fpy->operator()(time, 0), fpz->operator()(time, 0));
  };
  fclose(file_vpotential);
  file_vpotential = NULL;
}
void logConfig()
{
  logAdd((" [Command run: " + orgArgv + "]\n").c_str());
  logAdd(" |-------------------------------------------------------------------------\n");
  logAdd(" |PROJECT NAME: \"%s\".\n", STRINGIFY(PROJNAME));
  logAdd(" |-> EXPERIMENT NAME: \"%s\".\n", exp_name.c_str());
  logAdd(" |----> DEFAULT CONFIG FILE: \"%s\".\n", (defu_file + confext).c_str());
  logAdd(" |----> CONFIG FILE: \"%s\".\n", (conf_file + confext).c_str());
  // logAdd(" |\n");
  logAdd(" |       |-> PARAM FILES LOG\n");
  for (int i = 0; i < para->paramNotes.size(); i++)
  {
    logAdd(" |       |-> %s\n", para->paramNotes[i].c_str());
  }
  logAdd(" |----> COMMAND LINE OVERLOADS\n");
  for (int i = 0; i < cmdLineNotes.size(); i++)
  {
    logAdd(" |----> %s\n", cmdLineNotes[i].c_str());
  }
#ifndef IM
  string indent = " |         ";
  logAdd(" |----> VECTOR POTENTIALS\n");
  logAdd(" |        X-direction\n");
  logAdd("%s\n", vecpot_x->desc(indent).c_str());
  logAdd(" |        Y-direction\n");
  logAdd("%s\n", vecpot_y->desc(indent).c_str());
  logAdd(" |        Z-direction\n");
  logAdd("%s\n", vecpot_z->desc(indent).c_str());

#endif
  logAdd(" |FINAL NOTES:\n");
  double kmax = M_PI / delta_r;
  logAdd(" |-> Maximal momentum allowed by dx in this grid is kmax = %5.5le\n", kmax);
  kmax = kmax * 0.5;
  double kmax_d = kmax - pow3(kmax) * delta_r * delta_r / 6.0 - pow3(kmax) * pow2(kmax) * dt * dt / 16.0;
  logAdd(" |-> Numerical dispersion changes group velocity of kmax/2 = %5.5le\n", kmax_d);
  logAdd(" |-------------------------------------------------------------------------\n");
}
void logSilent(grid &g)
{
  //TODO: In future this will be the only thing done by logSilent

  if (file_log == NULL)
    return;
#ifdef IM
  fprintf(file_log, "Imaginary-time propagation\n");
  fprintf(file_log, "Grid: \n");
  fprintf(file_log, "g.ngps_x() = %ld\n", g.ngps_x());
  fprintf(file_log, "g.ngps_y() = %ld\n", g.ngps_y());
  fprintf(file_log, "g.ngps_z() = %ld\n", g.ngps_z());
  fprintf(file_log, "g.dimens() = %d\n\n", g.dimens());
  fprintf(file_log, "g.delt_x() = %20.15le\n", g.delt_x());
  fprintf(file_log, "nuclear_charge    = %20.15le\n", nuclear_charge);
  fprintf(file_log, "imag_timestep     = %20.15le\n", imag_timestep);
  fprintf(file_log, "max_steps         = %ld\n", max_steps);
  fprintf(file_log, "im_fname_log = %s\n", im_fname_log.c_str());
  for (int nr1 = 0; nr1 < max_n_number - l_qnumber; nr1++)
  {
    fprintf(file_log, "im_fname_wfs[%d] = %s\n", nr1, im_fname_wfs[nr1].c_str());
  }
#endif
#ifdef RE
  fprintf(file_log, "Real-time propagation\n");
  fprintf(file_log, "Grid: \n");
  fprintf(file_log, "  g_prop.dimens() = %d\n\n", g.dimens());
  fprintf(file_log, "  g_prop.ngps_x() = %ld\n", g.ngps_x());
  fprintf(file_log, "  g_prop.ngps_y() = %ld\n", g.ngps_y());
  fprintf(file_log, "  g_prop.ngps_z() = %ld\n", g.ngps_z());
  fprintf(file_log, "  g_prop.delt_x() = %15.10le\n", g.delt_x());
  fprintf(file_log, "delta_t        = %15.10le\n", dt);
  fprintf(file_log, "nuclear_charge = %15.10le\n", nuclear_charge);
  fprintf(file_log, "re_fname_wf    = %s\n", re_fname_wf.c_str());
  fprintf(file_log, "re_fname_observer  = %s\n", re_fname_observer.c_str());

  fprintf(file_log, "pulse-duration = %15.10le\n", pulse_duration);
  fprintf(file_log, "duration       = %15.10le\n", duration);
  fprintf(file_log, "Ip             = %15.10le\n", I_p);
  fprintf(file_log, "Up             = %15.10le\n", U_p);
  fprintf(file_log, "quiver-ampl    = %15.10le\n", quiver_amplitude);
  fprintf(file_log, "gamma_K        = %15.10le ", gamma_K);
  if (gamma_K > 1.0)
  {
    fprintf(file_log, "(multi-photon regime)\n");
  }
  else
  {
    fprintf(file_log, "(tunneling regime)\n");
  };
#endif
#ifdef IS
  fprintf(file_log, "Grid: \n");
  fprintf(file_log, "g.ngps_x()=%ld\n", g.ngps_x());
  fprintf(file_log, "g.ngps_y()=%ld\n", g.ngps_y());
  fprintf(file_log, "g.ngps_z()=%ld\n", g.ngps_z());
  fprintf(file_log, "g.dimens()=%d\n", g.dimens());
  fprintf(file_log, "g.delt_x()=%20.15le\n", g.delt_x());
  fprintf(file_log, "nuclear_charge   =%20.15le\n", nuclear_charge);
  fprintf(file_log, "re_fname_wf=%s\n", (dir_name + re_fname_wffinal).c_str());
#endif
  fflush(file_log);
}

void logPotential(hamop &hamilton)
{
  if (file_potential == NULL)
    return;

  for (long ir = 0; ir < long(im_radial_grid_size / delta_r); ir++)
  {
    const double R = double(ir) * delta_r;
    // calculate the total energy
    const double U = hamilton.scalarpot(R, l_qnumber, 1.0, 0.0, 0.0);
    fprintf(file_potential, "%20.15le %20.15le \n", R, U);
  };
  fflush(file_potential);
  fclose(file_potential);
  file_potential = NULL;
}

void stop_log(int proc = 0)
{
  if (proc == 0)
    logAdd("The results can be found in %s directory.\n", dir_name.c_str());
  if (file_log != NULL)
    fclose(file_log);
  if (file_potential != NULL)
    fclose(file_potential);
  if (file_vpotential != NULL)
    fclose(file_vpotential);
  if (file_observer != NULL)
    fclose(file_observer);
}
void printHelp(const option long_opts[])
{
  std::cout << " |------------------Help on using terminal parameters----------------------\n"
               " |\n"
               " | Use '--parameterName value' or '-parameterName value'.\n"
               " |\n"
               " | Note: Improper parameter name can result in parsing it improperly\n"
               " | example: using '-nuclear' instead of '-nuclear-charge' or '-Z' \n"
               " | will be interpretted as option '-n uclear' and error.\n"
               " |\n"
               " | Available options and their shortcut literals: \n";

  int i = 0;
  while (long_opts[i].name != nullptr)
  {
    std::cout << " | --" << long_opts[i].name << " or -" << (char)long_opts[i].val << endl;
    i++;
  }
  cout << " |-------------------------------------------------------------------------\n";

  exit(1);
}
void addCmdLineNotes(string s)
{
  cmdLineNotes.push_back(s);
}

void getCustomConfig(int argc, char *argv[], const option long_opts[], const char *short_opts)
{
  int opt;
  int index = 0;
  while (true)
  {
    opt = getopt_long(argc, argv, short_opts, long_opts, &index);
    if (-1 == opt)
      break;

    switch (opt)
    {
    case 'C':
      conf_file = string(optarg);
      cout << "Custom param file supplied, conf_file = " << conf_file << confext << endl;
      break;
    case 'U':
      defu_file = string(optarg);
      cout << "Custom defaults param file supplied, defu_file = " << defu_file << confext << endl;
      break;

    default:
      break;
    };
  };
}
void saveOrgOptions(int argc, char *argv[])
{
  for (int i = 0; i < argc; i++)
  {
    orgArgv += string(argv[i]) + " ";
  }
}

int processOptions(int argc, char *argv[])
{
  saveOrgOptions(argc, argv);

  // the colon (:) to indicate that the preceeding letter needs a parameter and is not a switch.
  // put ':' at the starting of the string so compiler can distinguish between '?' and ':'
  // More info: https://stackoverflow.com/questions/39966025/understanding-option-long-options-when-using-getopt-long
  const option long_opts[] = {
      // {"quiet", no_argument, nullptr, 'q'},
      {"help", no_argument, nullptr, 'h'},
      //Usefull for debugging
      {"verbose", no_argument, nullptr, 'v'},
      //Redirect all output to it's own directory under the project name directory
      {"exp-name", required_argument, nullptr, 'e'},

      //IM
      {"nuclear-charge", required_argument, nullptr, 'Z'},
      {"max-n-qnumber", required_argument, nullptr, 'N'},
      {"l-qnumber", required_argument, nullptr, 'L'},
      {"m-qnumber", required_argument, nullptr, 'M'},
      {"im-radial-grid-size", required_argument, nullptr, 'G'},
      //RE
      {"initial-n", required_argument, nullptr, 'n'},
      {"initial-l", required_argument, nullptr, 'l'},
      {"initial-m", required_argument, nullptr, 'm'},
      {"re-radial-grid-size", required_argument, nullptr, 'g'},
      {"l-grid-size", required_argument, nullptr, 'y'},
      //SPACETIME
      {"delta-r", required_argument, nullptr, 'r'},
      {"delta-t", required_argument, nullptr, 't'},

      //PULSE
      //Use for simple rescaling of the enabled fields
      {"intensity-mult", required_argument, nullptr, 'i'},
      {"delay-1", required_argument, nullptr, 'd'},
      {"delay-2", required_argument, nullptr, 'D'},
      {"omega-1", required_argument, nullptr, 'o'},
      {"omega-2", required_argument, nullptr, 'O'},
      {"im_extraid", required_argument, nullptr, 'x'},
      {"re_extraid", required_argument, nullptr, 'X'},
      //Not available through short flags
      // {"E1-x", required_argument, nullptr, 200},
      // {"E2-x", required_argument, nullptr, 201},
      // {"E1-y", required_argument, nullptr, 202},
      // {"E2-y", required_argument, nullptr, 203},
      // {"E1-z", required_argument, nullptr, 204},
      // {"E2-z", required_argument, nullptr, 205},
      // {"num-cycles-1-x", required_argument, nullptr, 300},
      // {"num-cycles-2-x", required_argument, nullptr, 301},
      // {"num-cycles-1-y", required_argument, nullptr, 302},
      // {"num-cycles-2-y", required_argument, nullptr, 303},
      // {"num-cycles-1-z", required_argument, nullptr, 304},
      // {"num-cycles-2-z", required_argument, nullptr, 305},
      // {"phase-1-x", required_argument, nullptr, 400},
      // {"phase-2-x", required_argument, nullptr, 401},
      // {"phase-1-y", required_argument, nullptr, 402},
      // {"phase-2-y", required_argument, nullptr, 403},
      // {"phase-1-z", required_argument, nullptr, 404},
      // {"phase-2-z", required_argument, nullptr, 405},

      // //TSURFF
      // {"additional-time", required_argument, nullptr, 'a'},
      // {"R-tsurff", required_argument, nullptr, 'T'},'
      // {"expansion-scheme", required_argument, nullptr, 's'},
      // {"k-min-surff", required_argument, nullptr, 'k'},
      // {"k-max-surff", required_argument, nullptr, 'K'},

      //Set a custom input conf file for options (other than config.param)
      //Ex. '--conf_file file' or just '-C file'
      {"conf_file", required_argument, nullptr, 'C'},
      //You can even change the default conf file (default-config.param)
      {"defu_file", required_argument, nullptr, 'U'},
      {"re_continue", no_argument, nullptr, 'A'},
      {nullptr, no_argument, nullptr, 0}};

  const char *const short_opts = "hvAe:Z:N:L:M:G:n:l:m:g:y:r:t:i:d:D:o:O:C:U:";
  //Check if new conf and default conf files were passed

  getCustomConfig(argc, argv, long_opts, short_opts);

  //Initialize conf files
  loadParams(conf_file, defu_file);
  //To reload getopt_long
  optind = 1;
  int opt;
  int index = 0;
  string str_ovr = "Terminal parameter override with ";
  // index works only if long option is given, so one must
  // put it by hand...
  while (true)
  {
    opt = getopt_long(argc, argv, short_opts, long_opts, &index);

    if (-1 == opt)
      break;

    switch (opt)
    {
      //Ignore config files during the second run
    case 'C':
    case 'U':
      break;
    case 'h': // -h or --help
      printHelp(long_opts);
      break;
    case 'v': // -h or --help
      verbose = 1;
      para->updateMergedParamField(string(long_opts[1].name), string(optarg), "bool");
      break;
    case 'e':
      exp_name = string(optarg);
      para->updateMergedParamField(string(long_opts[2].name), string(optarg));
      // addCmdLineNotes(str_ovr + string(long_opts[2].name) + " = " + string(optarg));
      break;
    case 'Z':
      nuclear_charge = stof(optarg);
      para->updateMergedParamField(string(long_opts[3].name), string(optarg));
      // addCmdLineNotes(str_ovr + string(long_opts[3].name) + " = " + string(optarg));
      // cout << str_ovr << long_opts[3].name << " = " << optarg << endl;
      break;
    case 'N':
      max_n_number = stoi(optarg);
      para->updateMergedParamField(string(long_opts[4].name), string(optarg));
      break;
    case 'L':
      l_qnumber = stoi(optarg);
      para->updateMergedParamField(string(long_opts[5].name), string(optarg));
      break;
    case 'M':
      m_qnumber = stoi(optarg);
      para->updateMergedParamField(string(long_opts[6].name), string(optarg));
      break;
    case 'G':
      im_radial_grid_size = stof(optarg);
      para->updateMergedParamField(string(long_opts[6].name), string(optarg));
      break;
    case 'n':
      re_initial_n = stoi(optarg);
      para->updateMergedParamField(string(long_opts[8].name), string(optarg));
      break;
    case 'l':
      re_initial_l = stoi(optarg);
      para->updateMergedParamField(string(long_opts[9].name), string(optarg));
      break;
    case 'm':
      re_initial_m = stoi(optarg);
      para->updateMergedParamField(string(long_opts[10].name), string(optarg));
      break;
    case 'g':
      re_radial_grid_size = stof(optarg);
      para->updateMergedParamField(string(long_opts[11].name), string(optarg));
      break;
    case 'y':
      re_l_grid_size = stoi(optarg);
      para->updateMergedParamField(string(long_opts[12].name), string(optarg));
      break;
    case 'r':
      delta_r = stof(optarg);
      para->updateMergedParamField(string(long_opts[13].name), string(optarg));
      break;
    case 't':
      dt = stof(optarg);
      para->updateMergedParamField(string(long_opts[14].name), string(optarg));
      break;
    case 'i':
      intensity_mult = stof(optarg);
      para->updateMergedParamField(string(long_opts[15].name), string(optarg));
      break;
    case 'd':
      delay1 = stof(optarg);
      para->updateMergedParamField(string(long_opts[16].name), string(optarg));
      break;
    case 'D':
      delay2 = stof(optarg);
      para->updateMergedParamField(string(long_opts[17].name), string(optarg));
      break;
    case 'o':
      omega1 = stof(optarg);
      para->updateMergedParamField(string(long_opts[18].name), string(optarg));
      break;
    case 'O':
      omega2 = stof(optarg);
      para->updateMergedParamField(string(long_opts[19].name), string(optarg));
      break;
    case 'x':
      im_extraid = string(optarg);
      para->updateMergedParamField(string(long_opts[20].name), string(optarg), string("string"));
      break;
    case 'X':
      re_extraid = string(optarg);
      para->updateMergedParamField(string(long_opts[21].name), string(optarg), string("string"));
      break;
    case 'A':
      logAdd("re_continue is turned on!\n");
      re_continue = true;
      break;
    case ':':
      addCmdLineNotes("Possible problem ':' recieved");
      // cout << ": " << (char)opt << " " << index << endl;
      break;
    case '?': // Unrecognized option
      // cout << "?" << endl;
      printHelp(long_opts);
      // if (optopt)
      // printf("The supplied short option '%c' goes unparsed. Add -h to see defined options\n", optopt);
      // else
      // printf("The supplied long option '%c' \"%s\" is not parsed by default\n", (char)opt, argv[optind]);
      // cout << "? " << endl;
      break;
    default:
      // cout << "default" << endl;
      // printf("The supplied (weird) option %i \"%s\" is not parsed by default\n", index, argv[optind]);
      addCmdLineNotes("Possible problem: default visted");
      // cout << "def " << optopt << " " << optarg << endl;
      break;
    };
  };
  // cout << "OPT IND " << optind << endl;
  return optind;
}

long int unix_timestamp()
{
  time_t t = std::time(0);
  long int now = static_cast<long int>(t);
  return now;
}

double always_zero2(double t, int me)
{
  return 0;
}

double always_zero5(double x, double y, double z, double t, int me)
{
  return 0;
}

void print_banner(string text)
{
  fprintf(stdout, " --------------------------------------------------------\n");
  fprintf(stdout, " %s.\n", text.c_str());
  fprintf(stdout, " (C) Copyright by Tulsky V A (2020), Bauer D and Koval P, Heidelberg (2005)\n");
  fprintf(stdout, " version modified and restructured by M. Mandrysz (2020)\n");
  fprintf(stdout, " --------------------------------------------------------\n\n");
}