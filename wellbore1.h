// Calculate the model for the well trajectory
// Compare with the existing model

//////////////////////////////////////////////////////////////////////// Check all the double , avoid the / 出错@！！！！1


#pragma once
#include <math.h>
#include <iostream>
#include <stdlib.h>

using namespace std;
double PI = 3.1415926;

//Declare function!
// Delta flow monitoring and PID control
double Delta_flow(double Q_injection, double Q_outflow);
// Pressure function
double wellboreAnnulus_pressure_drop(double Length, double D_po, double D_to, double L_tooljoint, double stdoff, double m, double D_hl, double e_den, double t_y, double K, double Q, double ToolJoint, double Roughness_wellbore, double Tortuosity);
double wellboreDrillpipe_pressure_drop(double m, double D_pi, double e_den, double t_y, double K, double Q);
// Temperature profile function
double wellboreAnnulus_temperature_drop();
double wellboreDrillpipe_temperature_drop();
// Lost circulation function
double fracture_loss_model(double A_frac_initial, double m, double Q, double pressure_wellbore);


// function body!
double wellboreAnnulus_pressure_drop(double Length, double D_po, double D_to,  double L_tooljoint, double stdoff, double m, double D_hl, double e_den, double t_y, double K, double Q, double ToolJoint, double Roughness_wellbore, double Tortuosity)
{
	double pressure_drop_tooljoint = 0;
	double fri_P_drop = 0;

	if (ToolJoint)
	{
		// add the computational code of the tool joint !!!!
		// input the information of tool joint !!!!
		// 1. pressure drop due to the narrow part of the tool joint

		double w = PI * (D_hl + D_to) / 2;
		double h = (D_hl - D_to) / 2;
		double A_anu_tool = w * h;
		// calculate the wall shear stress
		double v_av_tool = Q / A_anu_tool;                                       // average velocity
		double r_s = ((1 + 2 * m) / 3 / m)*(12 * v_av_tool) / (D_hl - D_to);     // shear rate  the first oder approximation
		double t_w1 = t_y + K * pow(r_s, m);                // initial t_w1
		double e = 1;
		// calculate the wall shear stress
		do
		{
			double x = t_y / t_w1;
			double Ca = 1 - x / (1 + m) - m* pow(x, 2) / (1 + m);
			double t_w2 = t_y + K * pow((r_s / Ca), m);
			e = t_w2 - t_w1;
			t_w1 = t_w2;
		} while (e > 0.00000001);

		double x = t_y / t_w1;
		double Ca = 1 - x / (1 + m) - m* pow(x, 2) / (1 + m);
		double N = m * Ca / (1 + 2 * m * (1 - Ca));
		// Geometry index of the pipe and wellbore    this is the diameter ratio
		double k = D_to / D_hl;
		//std::cout << " ***********************diameter ratio of the wellbore and tooljoint " << k << endl;
		// Critical Reynolds number for laminar flow
		double Re1 = 2100 * (pow(N, 0.331)*(1 + 1.402*k - 0.977*pow(k, 2)) - 0.019 *stdoff *pow(N, -0.868)*k);
		// Critical Reynolds number for turbulent flow
		double Re2 = 2900 * (pow(N, (-0.039*pow(Re1, 0.307))));
		// Reynolds number for Yield Power Law flow
		double Re_YPL = 12 * e_den * pow(v_av_tool, 2) / t_w1;
		// Laminar flow friction factor
		double f_Lam = 24 / Re_YPL;
		double f_1 = f_Lam;

		// calculate the  flow friction factor
		if (Re1 > Re_YPL)        // for laminar flow
		{
			if (stdoff != 0)
			{
				double R = (1 - 0.072*stdoff / N*pow(k, 0.8454) - 1.5*pow(stdoff, 2)*sqrt(N)*pow(k, 0.1852) + 0.96*pow(stdoff, 3)*sqrt(N)*pow(k, 0.2527));    // PLR pressure loss ratio
				f_1 = f_Lam * R;
			}
		}
		else                    // for turbulent and transient flow
		{
			if (Re_YPL > Re2)    // calculate the turbulent flow friction factor
			{
				// how to calculate the friction factor with Colebrook correlation.
				double rough = 0.003;   // roughness  degree !!!
				double D_eq = sqrt(pow(D_to, 2) - pow(D_hl, 2));       // equivalent  diameter 
				f_1 = 0.001;
				double abc = 100;
				do
				{
					double right = 1.14 - 2 * log(rough / D_eq + 9.34 / (Re_YPL*pow(f_1, 0.5)));              // A trial and error solution was used to solve colebrook equation with finning friction factor
					double f2 = pow(1 / right, 2);
					abc = fabs(f2 - f_1);
					f_1 = f2;
				} while (abc > 1e-6);
				if (stdoff)
				{
					double R = (1 - 0.048*stdoff / N*pow(k, 0.8454) - 0.667*pow(stdoff, 2)*sqrt(N)*pow(k, 0.1852) + 0.258*pow(stdoff, 3)*sqrt(N)*pow(k, 0.2527));    // PLR pressure loss ratio
					f_1 = f_1*R;
				}
			}
			if (Re_YPL > Re1 && Re_YPL < Re2)         // calculate the transitional fluid flow friction factor
			{
				// calculate the turbulent friction factor with Colebrook correlation.
				double rough = 0.003;   // roughness  degree !!!
				double D_eq = sqrt(pow(D_to, 2) - pow(D_po, 2));       // equivalent  diameter 
				double f_tur = 0.001;
				double abc = 100;
				do
				{
					double right = 1.14 - 2 * log(rough / D_eq + 9.34 / (Re_YPL*pow(f_tur, 0.5)));              // A trial and error solution was used to solve colebrook equation with finning friction factor
					double f2 = pow(1 / right, 2);
					abc = fabs(f2 - f_tur);
					f_tur = f2;
				} while (abc > 1e-6);
				double f_tran = f_Lam + (Re_YPL - Re1)*(f_tur - f_Lam) / (Re2 - Re1);
				f_1 = f_tran;
			}
		}

		// chech the Reynold's number in the narrow slot!!!!!!!!!!!!!!!!!!!!

		double pressure_drop_tooljoint1 = 2 * f_1 * e_den * pow(v_av_tool, 2) * L_tooljoint / (D_hl - D_to);
	/*	cout << "--------------------------------- the pressure drop along the tooljoint 1  " << pressure_drop_tooljoint1 << endl;*/

		// 2. pressure drop due to expansion and contraction

		double A_N = PI / 4 * (pow(D_hl, 2) - pow(D_to, 2));                   // narrow annulus area
		double A_W = PI / 4 * (pow(D_hl, 2) - pow(D_po, 2));
		double theta = 18;                                         // translate the rad/s   

		double Ra = ( D_po) / (D_to);
		
	/*	cout << Ra << "***************************** " << endl;*/
		// double check!   the ratio
		double K_c = 0.5 * sqrt(sin(theta / 2)*(1 - pow(Ra, 2)));
		double K_e = pow((1 - A_N / A_W), 2);

		double pressure_drop_tooljoint2 = e_den / 2 / 9.8* pow(v_av_tool, 2)*(K_c + K_e* pow((A_N / A_W), 2));
	/*	cout << "--------------------------------- the pressure drop along the tooljoint 2  " << pressure_drop_tooljoint2 << endl;*/

		// pressure drop due to the tooljoint
		pressure_drop_tooljoint = pressure_drop_tooljoint1 + pressure_drop_tooljoint2;
		/*cout << "--------------------------------- the pressure drop along the tooljoint " << pressure_drop_tooljoint << endl;*/
	}

	//*********************************************************************************************** narrow slot approximation *************************************************************************************

	if (ToolJoint == 0)
	{
		L_tooljoint = 0;
	}

	// based on narrow slot approximation
	double w = PI*(D_hl + D_po) / 2;
	double h = (D_hl - D_po) / 2;
	double A_anu = w * h;                                // annulus area
	// calculate the wall shear stress
	double v_av = Q / A_anu;                            // average velocity based on narrow slot approximation
	double r_s = ((1 + 2 * m) / 3 / m)*(12 * v_av) / (D_hl - D_po);     // initial shear rate  assume m = N; 
	double t_w1 = t_y + K * pow(r_s, m);                // initial t_w1
	// std::cout << " wall shear stress " << t_w1 << endl;
	double e = 1;
	// calculate the wall shear stress
	do
	{
		double x = t_y / t_w1;
		double Ca = 1 - x / (1 + m) - m* pow(x, 2) / (1 + m);
		double t_w2 = t_y + K * pow((r_s / Ca), m);
		e = t_w2 - t_w1;
		t_w1 = t_w2;
	} while (e > 0.00000001);

	// Calculate Generalized flow-behavior index. (Ahmed and Miska 2009)

	double x = t_y / t_w1;
	double Ca = 1 - x / (1 + m) - m* pow(x, 2) / (1 + m);
	double N = m * Ca / (1 + 2 * m * (1-Ca));

	// Use the correlation to calculate the critical Reynolds number for laminar and turbulent flow

	// Geometry index of the pipe and wellbore    this is the diameter ratio
	double k = D_po / D_hl;   
	//std::cout << " ***********************diameter ratio of the wellbore and drill pipe " << k << endl;
	// Critical Reynolds number for laminar flow
	double Re1 = 2100 * (pow(N, 0.331)*(1 + 1.402*k - 0.977*pow(k, 2)) - 0.019 *stdoff *pow(N, -0.868)*k);
	// Critical Reynolds number for turbulent flow
	double Re2 = 2900 * (pow(N, (-0.039*pow(Re1, 0.307))));
	// Reynolds number for Yield Power Law flow
	double Re_YPL = 12 * e_den * pow (v_av, 2) / t_w1;
	// Laminar flow friction factor
	double f_Lam = 24 / Re_YPL;
	//cout << " the frictional factor " << f_Lam << endl;
	//cout << " critical reynol's number " << Re1 << endl;
	//cout << " the reynold's number " << Re_YPL << endl;
	//cout << " the turbulent reynold's number " << Re2 << endl;
	double f_1 = f_Lam;

	/*cout << "frictional facotor for laminar fluid" << f_1 << endl;*/
	// calculate the  flow friction factor
	if (Re1 > Re_YPL)        // for laminar flow
	{
		if (stdoff != 0)
		{
			double R = (1 - 0.072*stdoff / N*pow(k, 0.8454) - 1.5*pow(stdoff, 2)*sqrt(N)*pow(k, 0.1852) + 0.96*pow(stdoff, 3)*sqrt(N)*pow(k, 0.2527));    // PLR pressure loss ratio
			f_1 = f_Lam * R;
			/*cout << "pressure loss ratio" <<R << endl;*/
		}
	}
	else                    // for turbulent and transient flow
	{
		if (Re_YPL > Re2)    // calculate the turbulent flow friction factor
		{
			// how to calculate the friction factor with Colebrook correlation.
			double rough = 0.003;   // roughness  degree !!!
			double D_eq = sqrt(pow(D_hl, 2) - pow(D_po, 2));       // equivalent  diameter 
			f_1 = 0.001;
			double abc = 100;
			do
			{
				double right = 1.14 - 2 * log(rough / D_eq + 9.34 / (Re_YPL*pow(f_1, 0.5)));              // A trial and error solution was used to solve colebrook equation with finning friction factor
				double f2 = pow(1 / right, 2);
				abc = fabs(f2 - f_1);
				f_1 = f2;
			} while (abc > 1e-6);
			if (stdoff)
			{
				double R = (1 - 0.048*stdoff / N*pow(k, 0.8454) - 0.667*pow(stdoff, 2)*sqrt(N)*pow(k, 0.1852) + 0.258*pow(stdoff, 3)*sqrt(N)*pow(k, 0.2527));    // PLR pressure loss ratio
				f_1 = f_1*R;
			}
		}
		if (Re_YPL > Re1 && Re_YPL < Re2)         // calculate the transitional fluid flow friction factor
			{
				// calculate the turbulent friction factor with Colebrook correlation.
				double rough = 0.003;   // roughness  degree !!!
				double D_eq = sqrt(pow(D_hl, 2) - pow(D_po, 2));       // equivalent  diameter 
				double f_tur = 0.001;
				double abc = 100;
				do
				{
					double right = 1.14 - 2 * log(rough / D_eq + 9.34 / (Re_YPL*pow(f_tur, 0.5)));              // A trial and error solution was used to solve colebrook equation with finning friction factor
					double f2 = pow(1 / right, 2);
					abc = fabs(f2 - f_tur);
					f_tur = f2;
				} while (abc > 1e-6);
				double f_tran = f_Lam + (Re_YPL - Re1)*(f_tur - f_Lam) / (Re2 - Re1);
				 f_1 = f_tran;
			}
	}

	    fri_P_drop = 2 * f_1 * e_den * pow(v_av,2)* (Length - L_tooljoint) / (D_hl - D_po);
		/*std::cout << " ***********************************************the frictional pressure drop for annulus: " << fri_P_drop << endl;*/

	//////////////////////////////////Tool joint effect of the annulus pressure drop/////////////////////////

	///////////////////////////////// Tortuosity of the wellbore ///////////////////////////////////////////

	// use the well trajectory data.
	if (Tortuosity)
	{

	}

	//////////////////////////////// Take the roughness of the wellbore into consideration ////////////////////

	// use the caliper log
	if (Roughness_wellbore)
	{

	}

	double P_pressure = fri_P_drop + pressure_drop_tooljoint;
	return P_pressure;
}

double wellboreDrillpipe_pressure_drop(double m, double D_pi, double e_den, double t_y, double K, double Q_injection)
{
	//Step 1: Calculate the wall shear stress
	double A_pip = PI*pow(D_pi, 2) / 4 ;
	double v_pipe = Q_injection / A_pip;      
	double r_s_1 = 8 * v_pipe / D_pi;
	double t_w1 = t_y + K * pow((r_s_1), m);// average velocity in the pipe Assume m = N
	double e = 100;
	do
	{
		double N_middle = ((1 - 2 * m)*t_w1 + 3*m*t_y )/ m / (t_w1 - t_y) + 2 * m*(1 + m)*((1 + 2 * m)*pow(t_w1, 2) + m*t_y*t_w1) / (m*(1 + m)*(1 + 2*m)*pow(t_w1, 2) + 2*pow(m, 2)*(1 + m)*t_w1*t_y + 2*pow(m, 3)*pow(t_y, 2));
		double N = 1 / N_middle;
		double D_eff = 4 * N * D_pi / (3 * N + 1);      // effect diameter calculate the effect of non-Newtonian effect.
		double r_s_2 = 8 * v_pipe / D_eff;
		double t_w2 = t_y + K * pow((r_s_2), m);
		e = t_w1 - t_w2;
		t_w1 = t_w2;
		r_s_1 = r_s_2;
	} while (e > 0.000000001);
	std::cout << "the wall shear stress is " << t_w1 << endl;
	double u_app = t_w1 / r_s_1;    // the apparent viscosity
	double Re = e_den * v_pipe * D_pi / u_app;    // calculate the Reynold's number
	double f1 = 0;
	if (Re < 2100)
	{
		double f1 = 16 / Re;         // laminar flow frictional factor
	}
	else
	{
		// use the Colebrook correlation
		double rough = 0.003;     // roughness  degree !!!                 (!!!!this has a problem!!!!)
		double D_eq = D_pi;       // equivalent  diameter 
		double f_tur = 0.001;
		double abc = 100;
		do
		{
			double right = 1.14 - 2 * log(rough / D_eq + 9.34 / (Re*pow(f_tur, 0.5)));              // A trial and error solution was used to solve colebrook equation with finning friction factor
			double f2 = pow(1 / right, 2);
			abc = fabs(f2 - f_tur);
			f_tur = f2;
			f1 = f_tur;        // this is the fanning frictional factor
		} while (abc > 1e-6);
	}
	// calculate the frictional pressure drop along the pipe
	double t_w = f1*e_den*v_pipe / 2;
	double pressure_drop_pipe = 4 * t_w / D_pi;
	std::cout << "the pressure drop in pipe" << pressure_drop_pipe << endl;
	return pressure_drop_pipe;
}

double wellboreDrillpipe_temperature_drop(double e_den, double Q_injection, double Cp, double D_Pi, double U, double P_pressure, double pressure_drop_pipe, double T_dpi, double T_s, double ho, double G_thom, double Length)
{
	// Linear temperature profile in the wellbore, analytical solution of the temperature profile
	double r1, r2;    // check this line if it is correct or not!!!
	r1 = ((2 * PI * D_Pi/2 * ho) + sqrt((2 * PI * D_Pi/2 * ho)^2 + 4 * (2 * PI * D_Pi/2 * U) * (2 * PI * D_Pi/2 * ho))) / (2 * e_den * Q_injection * PI * ho );
	r1 = ((2 * PI * D_Pi/2 * ho) - sqrt((2 * PI * D_Pi/2 * ho)^2 + 4 * (2 * PI * D_Pi/2 * U) * (2 * PI * D_Pi/2 * ho))) / (2 * e_den * Q_injection * PI * ho );
	double A = e_den * Q_injection * Cp / (2 * PI * D_Pi/2 * U);
	double C1, C2;
	C1 = ((T_dpi - T_s - P_pressure/(2 * PI * D_Pi/2 * U) - (P_Pressure + pressure_drop_pipe)/(2 * PI* D_Pi/2 * ho) +A * G_thom) * (A * r2 * exp(r2 * Length)) - pressure_drop_pipe/(2 * PI * D_Pi/2 * U) + A * G_thom) ...
	/(A * r2 * exp(r2 * Length) - A * r1 * exp(r1 * Length));
	C2 = T_dpi - T_s - P_pressure/(2 * PI * D_Pi/2 * U) - (P_Pressure + pressure_drop_pipe)/(2 * PI * D_Pi/2 * ho) + A * G_thom - C1;
	double Tdpi = C1 + C2 + T_s + P_Pressure / (2 * PI * D_Pi/2 * U) + (P_Pressure + pressure_drop_pipe)/(2 * PI * D_Pi/2 * ho) + A * G_thom;
	return Tdpi;
}
double wellboerAnnulus_temperature_drop()
{
	
}
double Delta_flow(double Q_injection, double Q_outflow)
{
	// embeded PID controller! For Lost circulation

	Delta_flow = Q_injection - Q_outflow;
	return Delta_flow
}

double fracture_loss_model(double t_y, double K, double e_mix, double r, double w_i, double m, double Q_loss, double pressure_wellbore)
{

	///////////////// According to the Q change the radius//////////////////////
	      ///////////////// Caution r is changable//////////////////////

	double v_av_frac = Q_loss / 2/ PI / r/w_i;							          // average velocity   the Radius is changing !!! be ware of that!!!
	double r_s_fra = ((1 + 2 * m) / 4 / m)*(8 * v_av_frac) / pow(w_i,2);         // shear rate  at fracture  assume that m = N
	double t_w1 = t_y + K * pow(r_s_fra, m);								    // initial t_w1
	double e = 1;
	// calculate the wall shear stress
	do
	{
		double x = t_y / t_w1;
		double Ca = (1 - x / (1 + m) - m* pow(x, 2) / (1 + m)) * 2/ w_i;
		double t_w2 = t_y + K * pow((r_s_fra / Ca), m);
		e = t_w2 - t_w1;
		t_w1 = t_w2;
	} while (e > 0.00000001);
	// Calculate Generalized flow-behavior index. 
	std::cout << "wall shear stress is: " << t_w1 << endl;

	double x = t_y / t_w1;          // the eventually data from calculation.
	double Ca = (1 - x / (1 + m) - m* pow(x, 2) / (1 + m)) * 2 / w_i;
	double N = Ca*m / (1 + 2 * m*(1 - Ca));
	std::cout << "the generalized fluid flow index(0.15 < N < 0.4): " << N << endl;

	// Reynold's number of the calculation !!!!!!!!!!!!!!!!!!!!!!!!!!!!     check the Reynold's number, to see if we can get Turbulent Flow.

    r_s_fra = ((1 + 2 * N) / 4 / N)*(8 * v_av_frac) / pow(w_i, 2);
	double u_app = t_w1 / r_s_fra;
	// calculate the Reynolds number            according to this penny shape, and calculate the Reynold's number
	double D_hy = 4*PI*r*w_i / (w_i + 2*PI*r);
	double Re = e_mix * v_av_frac * D_hy / u_app;
	std::cout << " ***** the Reynold's number for the fracture is " << Re << endl;

	double f1 = 0;
	// calculate the frictional factor of the fracture
	if (Re < 2000)
	{
		f1 = 16 / Re;
	}
	else
	{
		// how to calculate the friction factor with Colebrook correlation.
		double rough = 0.0003;   // roughness  degree !!!
		double D_eq = w_i;       // equivalent  diameter 
		double f_1 = 0.001;
		double abc = 100;
		do
		{
			double right = 1.14 - 2 * log(rough / D_eq + 9.34 / (Re*pow(f_1, 0.5)));              // A trial and error solution was used to solve colebrook equation with finning friction factor
			double f2 = pow(1 / right, 2);
			abc = fabs(f2 - f_1);
			f_1 = f2;
		} while (abc > 1e-6);
	}
	// the pressure drop along the fracture can be calculated by
	double pressure_drop_fracture = 2 * f1 * e_mix * pow (v_av_frac, 2) / (w_i);
	return pressure_drop_fracture;
}