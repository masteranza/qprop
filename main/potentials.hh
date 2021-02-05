#ifndef __POTENTIALS_H__
#define __POTENTIALS_H__
#include <iomanip>
class vecpot
{
public:
  vecpot(){};
  //Returns the value of the vector potential at time t
  virtual double operator()(double time, int me) const { return 0.0; };
  virtual double get_duration() const { return 0; };
  virtual double get_Up() const { return 0; };
  virtual double get_quiver_amplitude() const { return 0; };
  virtual double integral(double time) const { return 0; };
  virtual string name() const { return ""; };
  virtual string desc(string indent) const { return ""; };
};

//Use when there's no vecpot interaction in some direction
class Zero_vecpot : public vecpot
{
public:
  Zero_vecpot() : vecpot(){};
  virtual string name() const { return "Zero"; };
  virtual string desc(string indent) const
  {
    std::ostringstream oss;
    oss << indent << setw(15) << right << "vecpot name: " << name() << "" << endl
        << indent << setw(15) << right << "E_1: " << setw(12) << left << 0.0 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "duration: " << setw(12) << left << get_duration() << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "quiver: " << setw(12) << left << get_quiver_amplitude() << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "U_p: " << setw(12) << left << get_Up() << setw(6) << "[au]";
    return oss.str();
  };
};

class Const_vecpot : public vecpot
{
protected:
  double E_1;
  double duration;

public:
  Const_vecpot(double E_max1, double dur) : E_1(E_max1), duration(dur){};
  virtual double operator()(double time, int me) const
  {
    double res;
    if (time < duration)
      res = -E_1 * time;
    else
      res = 0.0;
    return res;
  };
  virtual double get_duration() const
  {
    return duration;
  };
  virtual double get_Up() const
  {
    //Technicly should be infnity...
    return 0.0;
  };
  virtual double get_quiver_amplitude() const
  {
    //Technicly should be infnity...
    return 0.0;
  }
  virtual double integral(double time) const
  {

    double res = 0.0;
    if (time < duration)
    {
      res = -E_1 * time * time * 0.5;
    };
    return res;
  };
  virtual string name() const { return "Const"; };
  virtual string desc(string indent) const
  {
    std::ostringstream oss;
    oss << indent << setw(15) << right << "vecpot name: " << name() << "" << endl
        << indent << setw(15) << right << "E_1: " << setw(12) << left << E_1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "duration: " << setw(12) << left << get_duration() << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "quiver: " << setw(12) << left << get_quiver_amplitude() << "(inf) [au] " << endl
        << indent << setw(15) << right << "U_p: " << setw(12) << left << get_Up() << "(inf) [au]";
    return oss.str();
  };
};

// The vector potential with sine squared envelope
class SinEnvSin2_vecpot : public vecpot
{
protected:
  double omega1;
  double n_c1;
  double delay1;
  double E_1;
  double phi_cep1;
  double ww1;
  double duration1;
  double duration;
  double excursion_ampl1;
  double Up1;

public:
  SinEnvSin2_vecpot(double om1, double n_cyc1, double del1, double E_max1, double cep1 = 0.0) : omega1(om1), n_c1(n_cyc1), delay1(del1), E_1(E_max1), phi_cep1(cep1)
  {
    if (omega1 < 0.0)
      throw "Frequency omega1 can't be negative.";
    if (omega1 == 0.0 && E_1 != 0.0)
      throw "Frequency omega1 is zero while the field E_1 is not. Consider using a Const_vectpot or correct.";

    //
    if (n_c1 == 1.0)
    {
      cout << "Warning: one cycle pulses generate error in the vector potential integral. n_1 artifically increased to 1.01." << endl;
      n_c1 = 1.01;
    }
    delay1 = 2 * M_PI * delay1 / omega1;
    duration1 = n_c1 * 2 * M_PI / omega1 + delay1;
    duration = duration1;
    //Phase in units of 2*M_PI for convienence
    phi_cep1 *= 2 * M_PI;
    // Angular frequency of the envelope
    ww1 = omega1 / (2.0 * n_c1);
    //Quiver amplitude and ponderomotive energy
    excursion_ampl1 = E_1 / (omega1 * omega1);
    Up1 = 0.25 * (E_1 * E_1) / (omega1 * omega1);
  };
  double operator()(double time, int me) const
  {
    double result = 0.0;
    if (time >= delay1 && time < duration1)
    {
      result += E_1 / omega1 * pow2(sin(ww1 * (time - delay1))) * sin(omega1 * (time - delay1) + phi_cep1);
    };
    return result;
  };
  double get_duration() const
  {
    return duration;
  };
  double get_Up() const
  {
    return Up1;
  };
  double get_quiver_amplitude() const
  {
    return excursion_ampl1;
  }
  double integral(double time) const
  {
    double result = 0.0;
    if (time >= delay1 && time <= duration1)
    {
      result += 0.5 * excursion_ampl1 * (cos(phi_cep1) * (1.0 - 0.5 * n_c1 / (n_c1 + 1.0) - 0.5 * n_c1 / (n_c1 - 1.0)) - cos(omega1 * (time - delay1) + phi_cep1) + cos(omega1 * (time - delay1) + omega1 * (time - delay1) / n_c1 + phi_cep1) * 0.5 * n_c1 / (n_c1 + 1.0) + cos(omega1 * (time - delay1) - omega1 * (time - delay1) / n_c1 + phi_cep1) * 0.5 * n_c1 / (n_c1 - 1.0));
    };
    return result;
  };
  virtual string name() const { return "Sine with envelope sin^2"; };
  virtual string desc(string indent) const
  {
    std::ostringstream oss;
    oss << indent << setw(15) << right << "vecpot name: " << name() << "" << endl
        << indent << setw(15) << right << "E_1: " << setw(12) << left << E_1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "omega1: " << setw(12) << left << omega1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "n_c1: " << setw(12) << left << n_c1 << setw(6) << "cycles" << endl
        << indent << setw(15) << right << "phi_cep1: " << setw(12) << left << phi_cep1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "duration: " << setw(12) << left << get_duration() << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "delay1: " << setw(12) << left << delay1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "quiver: " << setw(12) << left << get_quiver_amplitude() << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "U_p: " << setw(12) << left << get_Up() << setw(6) << "[au]";
    return oss.str();
  };
};

class TwoSinEnvSin2_vecpot : public SinEnvSin2_vecpot
{
protected:
  double omega2;
  double n_c2;
  double delay2;
  double E_2;
  double phi_cep2;
  double ww2;
  double duration2;
  double excursion_ampl2;
  double Up2;

public:
  TwoSinEnvSin2_vecpot(double om1, double om2, double n_cyc1, double n_cyc2, double del1, double del2, double E_max1, double E_max2, double cep1 = 0.0, double cep2 = 0.0) : SinEnvSin2_vecpot(om1, n_cyc1, del1, E_max1, cep1), omega2(om2), n_c2(n_cyc2), delay2(del2), E_2(E_max2), phi_cep2(cep2)
  {
    if (omega2 < 0.0)
      throw "Frequency omega2 can't be negative.";
    if (omega2 == 0.0 && E_2 != 0.0)
      throw "Frequency omega2 is zero while the field E_2 is not. Consider using a SinEnvSin2_vecpot or correct.";

    if (n_c2 == 1.0)
    {
      cout << "Warning: one cycle pulses generate error in the vector potential integral. n_2 artifically increased to 1.01." << endl;
      n_c2 = 1.01;
    }
    delay2 = 2 * M_PI * delay2 / omega1;
    duration2 = n_c2 * 2 * M_PI / omega2 + delay2;
    duration = (duration1 > duration2) ? duration1 : duration2; // Total duration of the (overlaping) pulses
    //Phase in units of 2*M_PI for convienence
    phi_cep2 *= 2 * M_PI;
    // Angular frequency of the envelope
    ww2 = omega2 / (2.0 * n_c2);
    //Quiver amplitude and ponderomotive energy
    excursion_ampl2 = E_2 / (omega2 * omega2);
    Up2 = 0.25 * (E_2 * E_2) / (omega2 * omega2);
  };
  virtual double operator()(double time, int me) const
  {
    double result = 0.0;
    if (E_1 != 0.0 && time >= delay1 && time <= duration1)
    {
      result += E_1 / omega1 * pow2(sin(ww1 * (time - delay1))) * sin(omega1 * (time - delay1) + phi_cep1);
    };
    if ((E_2 != 0.0) && (time >= delay2) && (time <= duration2))
    {
      result += E_2 / omega2 * pow2(sin(ww2 * (time - delay2))) * sin(omega2 * (time - delay2) + phi_cep2);
    };
    return result;
  };
  virtual double get_duration() const
  {
    return duration;
  };
  virtual double get_Up() const
  {

    return max(Up1, Up2);
  };
  virtual double get_quiver_amplitude() const
  {

    return max(excursion_ampl1, excursion_ampl2);
  }
  virtual double integral(double time) const
  {

    // This is the time integral of A(t), i.e., the free-electron excursion alpha(t)
    double result = 0.0;
    if (E_1 != 0.0 && time >= delay1 && time <= duration1)
    {

      result += 0.5 * excursion_ampl1 * (cos(phi_cep1) * (1.0 - 0.5 * n_c1 / (n_c1 + 1.0) - 0.5 * n_c1 / (n_c1 - 1.0)) - cos(omega1 * (time - delay1) + phi_cep1) + cos(omega1 * (time - delay1) + omega1 * (time - delay1) / n_c1 + phi_cep1) * 0.5 * n_c1 / (n_c1 + 1.0) + cos(omega1 * (time - delay1) - omega1 * (time - delay1) / n_c1 + phi_cep1) * 0.5 * n_c1 / (n_c1 - 1.0));
      // result += 0.5 * excursion_ampl1 * (cos(phi_cep1) * (1.0 - 0.5 * n_c1 / (n_c1 + 1.0) - 0.5 * n_c1 / (n_c1 - 1.0)) - cos(omega1 * time + phi_cep1) + cos(omega1 * time + omega1 * time / n_c1 + phi_cep1) * 0.5 * n_c1 / (n_c1 + 1.0) + cos(omega1 * time - omega1 * time / n_c1 + phi_cep1) * 0.5 * n_c1 / (n_c1 - 1.0));
    };
    if (E_2 != 0.0 && (time >= delay2) && (time <= duration2))
    {
      result += 0.5 * excursion_ampl2 * (cos(phi_cep2) * (1.0 - 0.5 * n_c2 / (n_c2 + 1.0) - 0.5 * n_c2 / (n_c2 - 1.0)) - cos(omega2 * (time - delay2) + phi_cep2) + cos(omega2 * (time - delay2) + omega2 * (time - delay2) / n_c2 + phi_cep2) * 0.5 * n_c2 / (n_c2 + 1.0) + cos(omega2 * (time - delay2) - omega2 * (time - delay2) / n_c2 + phi_cep2) * 0.5 * n_c2 / (n_c2 - 1.0));
    };
    return result;
  };
  virtual string name() const { return "Two sines with envelope sin^2"; };
  virtual string desc(string indent) const
  {
    std::ostringstream oss;
    oss << indent << setw(15) << right << "vecpot name: " << name() << "" << endl
        << indent << setw(15) << right << "E_1: " << setw(12) << left << E_1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "E_2: " << setw(12) << left << E_2 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "omega1: " << setw(12) << left << omega1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "omega2: " << setw(12) << left << omega2 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "n_c1: " << setw(12) << left << n_c1 << setw(6) << "cycles" << endl
        << indent << setw(15) << right << "n_c2: " << setw(12) << left << n_c2 << setw(6) << "cycles" << endl
        << indent << setw(15) << right << "phi_cep1: " << setw(12) << left << phi_cep1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "phi_cep2: " << setw(12) << left << phi_cep2 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "duration1: " << setw(12) << left << duration1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "duration2: " << setw(12) << left << duration2 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "delay1: " << setw(12) << left << delay1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "delay2: " << setw(12) << left << delay2 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "duration: " << setw(12) << left << get_duration() << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "quiver1: " << setw(12) << left << excursion_ampl1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "quiver2: " << setw(12) << left << excursion_ampl2 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "quiver: " << setw(12) << left << get_quiver_amplitude() << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "Up1: " << setw(12) << left << Up1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "Up2: " << setw(12) << left << Up2 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "U_p: " << setw(12) << left << get_Up() << setw(6) << "[au]";
    return oss.str();
  };
  // // print alpha(t) calculated from the analytical expression and by trapezoidal rule (dt: time step for numerical quadrature, freq: write output every freq time steps)
  // void debugIntegral(double dt, long freq)
  // {
  //   std::ofstream debug_alpha("debug-alpha.dat");
  //   long steps(2 + duration / dt);
  //   double alpha_trapez(0.0);
  //   for (long i(1); i < steps; i++)
  //   {
  //     const double time = double(i) * dt;
  //     alpha_trapez += operator()(time, 0) * 0.5;
  //     if (i % freq == 0)
  //     {
  //       debug_alpha<<time<<" "<<alpha_trapez * dt<<" "<<integral(time)<<" "<<alpha_trapez * dt - integral(time)<<std::endl;
  //     };
  //     alpha_trapez += operator()(time, 0) * 0.5;
  //   };
  // };
};

class SinRampTwoCycleSin2_vecpot : public SinEnvSin2_vecpot
{
public:
  SinRampTwoCycleSin2_vecpot(double om1, double n_cyc1, double del1, double E_max1, double cep1 = 0.0) : SinEnvSin2_vecpot(om1, n_cyc1, del1, E_max1, cep1){};

  double operator()(double time, int me) const
  {
    double result = 0.0;
    if (E_1 != 0.0 && time >= delay1 && time <= duration1)
    {
      if ((time - delay1) <= 2 * 2 * M_PI / omega1)
        result += E_1 / omega1 * pow2(sin(0.125 * omega1 * (time - delay1))) * sin(omega1 * (time - delay1) + phi_cep1);
      else if ((time - delay1) < (n_c1 - 2.0) * 2 * M_PI / omega1)
        result += E_1 / omega1 * sin(omega1 * time + phi_cep1);
      else
        result += E_1 / omega1 * pow2(sin(0.125 * (omega1 * time - (n_c1 - 4.0) * 2 * M_PI))) * sin(omega1 * time + phi_cep1);
    }
    return result;
  };
  //TODO: Define integral() properly for this type of pulse
  virtual string name() const { return "Sine with two cycle sin^2 ramp"; };
  virtual string desc(string indent) const
  {
    std::ostringstream oss;
    oss << indent << setw(15) << right << "vecpot name: " << name() << "" << endl
        << indent << setw(15) << right << "E_1: " << setw(12) << left << E_1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "omega1: " << setw(12) << left << omega1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "n_c1: " << setw(12) << left << n_c1 << setw(6) << "cycles" << endl
        << indent << setw(15) << right << "phi_cep1: " << setw(12) << left << phi_cep1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "duration: " << setw(12) << left << get_duration() << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "delay1: " << setw(12) << left << delay1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "quiver: " << setw(12) << left << get_quiver_amplitude() << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "U_p: " << setw(12) << left << get_Up() << setw(6) << "[au]" << endl;
    return oss.str();
  };
};

class TwoSinRampTwoCycleSin2_vecpot : public TwoSinEnvSin2_vecpot
{
public:
  TwoSinRampTwoCycleSin2_vecpot(double om1, double om2, double n_cyc1, double n_cyc2, double del1, double del2, double E_max1, double E_max2, double cep1 = 0.0, double cep2 = 0.0) : TwoSinEnvSin2_vecpot(om1, om2, n_cyc1, n_cyc2, del1, del2, E_max1, E_max2, cep1, cep2){};

  double operator()(double time, int me) const
  {
    double result = 0.0;
    if (E_1 != 0.0 && time >= delay1 && time <= duration1)
    {
      // Here's the shape of the laser pulse
      if ((time - delay1) <= 2 * 2 * M_PI / omega1)
        result += E_1 / omega1 * pow2(sin(0.125 * omega1 * (time - delay1))) * sin(omega1 * (time - delay1) + phi_cep1);
      else if ((time - delay1) < (n_c1 - 2.0) * 2 * M_PI / omega1)
        result += E_1 / omega1 * sin(omega1 * (time - delay1) + phi_cep1);
      else
        result += E_1 / omega1 * pow2(sin(0.125 * (omega1 * (time - delay1) - (n_c1 - 4.0) * 2 * M_PI))) * sin(omega1 * (time - delay1) + phi_cep1);
    };
    if (E_2 != 0.0 && (time >= delay2) && (time <= duration2))
    {
      if (time <= delay2 + 2 * 2 * M_PI / omega2)
        result += E_2 / omega2 * pow2(sin(0.125 * omega2 * (time - delay2))) * sin(omega2 * (time - delay2) + phi_cep2);
      else if (time < delay2 + (n_c2 - 2.0) * 2 * M_PI / omega2)
        result += E_2 / omega2 * sin(omega2 * (time - delay2) + phi_cep2);
      else
        result += E_2 / omega2 * pow2(sin(0.125 * (omega2 * (time - delay2) - (n_c2 - 4.0) * 2 * M_PI))) * sin(omega2 * (time - delay2) + phi_cep2);
    };
    return result;
  };
  //TODO: Define integral() properly for this type of pulse
  virtual string name() const { return "Two sines with 2 cycle sin^2 ramp"; };
  virtual string desc(string indent) const
  {
    std::ostringstream oss;
    oss << indent << setw(15) << right << "vecpot name: " << name() << "" << endl
        << indent << setw(15) << right << "E_1: " << setw(12) << left << E_1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "E_2: " << setw(12) << left << E_2 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "omega1: " << setw(12) << left << omega1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "omega2: " << setw(12) << left << omega2 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "n_c1: " << setw(12) << left << n_c1 << setw(6) << "cycles" << endl
        << indent << setw(15) << right << "n_c2: " << setw(12) << left << n_c2 << setw(6) << "cycles" << endl
        << indent << setw(15) << right << "phi_cep1: " << setw(12) << left << phi_cep1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "phi_cep2: " << setw(12) << left << phi_cep1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "duration1: " << setw(12) << left << duration1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "duration2: " << setw(12) << left << duration2 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "delay1: " << setw(12) << left << delay1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "delay2: " << setw(12) << left << delay2 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "duration: " << setw(12) << left << get_duration() << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "quiver1: " << setw(12) << left << excursion_ampl1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "quiver2: " << setw(12) << left << excursion_ampl2 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "quiver: " << setw(12) << left << get_quiver_amplitude() << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "Up1: " << setw(12) << left << Up1 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "Up2: " << setw(12) << left << Up2 << setw(6) << "[au]" << endl
        << indent << setw(15) << right << "U_p: " << setw(12) << left << get_Up() << setw(6) << "[au]";
    return oss.str();
  };
};

class scalarpot
{
protected:
  double nuclear_charge;

public:
  scalarpot(double charge) : nuclear_charge(charge){};

  virtual double operator()(double x, double y, double z, double time, int me) const
  {
    double result = -nuclear_charge / x;
    return result;
  };
  double get_nuclear_charge() const { return nuclear_charge; };
};

class CutCoulomb_scalarpot : public scalarpot
{
protected:
  double R_co;

public:
  CutCoulomb_scalarpot(double charge, double co) : scalarpot(charge), R_co(co){};

  virtual double operator()(double x, double y, double z, double time, int me) const
  {
    // Scrinzi potential
    // double result=(x<R_co)?nuclear_charge*(-1.0/x-pow2(x)/(2*pow3(R_co))+3.0/(2.0*R_co)):0.0;
    // Simple Volker potential; first -1/r then linear then zero
    const double m = 1.0 / pow2(R_co);
    double result = (x < R_co) ? -1.0 / x : ((x < 2 * R_co) ? -1.0 / R_co + m * (x - R_co) : 0.0);
    return nuclear_charge * result;
  };
};

//TODO: implement non-reflecting absorbing potential?
class imagpot
{
protected:
  long imag_potential_width;
  double ampl_im; // Amplitude of imaginary absorbing potential  <--------------  100.0 imag pot on,  0.0 off
public:
  imagpot(long width, double ampl = 100.0) : ampl_im(ampl), imag_potential_width(width){
                                                                //
                                                            };
  virtual double operator()(long xindex, long yindex, long zindex, double time, grid g) const
  {
    if (ampl_im > 1.0)
    {
      const long imag_start = g.ngps_x() - imag_potential_width;
      if (xindex < imag_start)
        return 0;
      else
      {
        const double r = double(xindex - imag_start) / double(imag_potential_width);
        return ampl_im * pow2(pow2(pow2(pow2(r))));
      };
    }
    else
    {
      return 0.0;
    };
  };
};
#endif