/*
 * sm_pms_est.c
 *
 *  Created on: Oct 18, 2025
 *      Author: Admin
 */

#include "sm_pms_est.h"
#include "sm_ev_mc_module.h"
#include "sm_ev_pmu_module.h"
#include "sm_sv_pms.h"
#define _impl(p)		((est_data_impl_t*)(p))

static est_data_t g_est_data;
est_data_t* sm_pms_est_create(sm_sv_bp_t *_bp, sm_bp_retain_t *_bp_reatain,
        sm_ev_module_t* mc_module, sm_ev_module_t* pmu_module, sys_energy _energy){

	est_data_t *this = &g_est_data;

	g_est_data.m_bpm = _bp;
	g_est_data.m_bp_reatain = _bp_reatain;
	g_est_data.mc_module = mc_module;
	g_est_data.pmu_module = pmu_module;

	g_est_data.m_elec_energycharge = _energy.elec_energyCharge;
	g_est_data.m_elec_energydischarge = _energy.elec_energyDischarge;
	return (est_data_t*) this;
}
//static void pms_est_total_energy(est_data_t *_this){
//    float total_energy = 0.0;
//    uint32_t soc_total = 0;
//    const sm_bp_data_t *bp_data = NULL;
//    for (uint8_t i= 0; i < SM_BP_NUMBER_DEFAULT; i++){
//        if(!sm_sv_bp_is_connected(_this->m_bpm, i)) {
//            continue;
//        }
//        bp_data = sm_sv_bp_get_data(_this->m_bpm, i);
//        soc_total += (uint32_t)bp_data->m_soc;
//    }
//    total_energy = (float) (soc_total - 5)*1160/100; /// Hardcode BP's energy : 1160Wh
//    _this->m_total_energy = (int32_t)total_energy;
//}

#define RPM_TO_RADS_RATIO		(0.10472f)

static void pms_est_energy(est_data_t *_this){


    sm_mc_data_t*mc_data = sm_mc_get_data(_this->mc_module);
    PMS_MODE pms_mode = sm_sv_pms_get_mode();
    const sm_bp_data_t *bp_data = NULL;
    if(pms_mode == PMS_DISCHARGER){
        for( uint8_t i = 0; i < SM_BP_NUMBER_DEFAULT; i++){
            if(sm_sv_bp_is_connected(_this->m_bpm, i)){
                bp_data = sm_sv_bp_get_data(_this->m_bpm, i);
                float vol = (float) bp_data->m_vol;
                float cur = (float) bp_data->m_cur;
                float mili_power = vol*cur/1000000;
                _this->m_elec_energydischarge += (uint32_t)(mili_power/7200);
            }
        }
    }
    else if(pms_mode == PMS_CHARGER){
        for( uint8_t i = 0; i < SM_BP_NUMBER_DEFAULT; i++){
            if(sm_sv_bp_is_connected(_this->m_bpm, i)){
                bp_data = sm_sv_bp_get_data(_this->m_bpm, i);
                float vol = (float) bp_data->m_vol;
                float cur = (float) bp_data->m_cur;
                float mili_power = vol*cur/(-1000000);
                _this->m_elec_energycharge += (uint32_t)(mili_power/7200);
            }
        }

    }
    float mechanical_power = mc_data->m_est_tor*mc_data->m_speed_rpm*RPM_TO_RADS_RATIO;
    _this->m_mechanical_energy += (uint32_t)(mechanical_power / 7200);
}
uint32_t OCV_table[101] = {
		53000,	53000,	53000,	53000,	53000,	53000,	55114,	55203,	55291,	55371,
		55440,	55520,	55619,	55763,	55962,	56146,	56512,	56656,	56774,	56890,
		57003,	57112,	57206,	57301,	57381,	57445,	57525,	57589,	57658,	57723,
		57778,	57827,	57877,	57915,	57960,	58005,	58050,	58090,	58139,	58179,
		58224,	58278,	58323,	58373,	58422,	58472,	58531,	58590,	58650,	58715,
		58784,	58858,	58942,	59037,	59155,	59299,	59469,	59662,	59845,	60010,
		60142,	60272,	60402,	60530,	60659,	60794,	60931,	61070,	61210,	61349,
		61493,	61642,	61790,	62093,	62246,	62405,	62563,	62723,	62882,	63050,
		63213,	63378,	63546,	63715,	63883,	64067,	64245,	64429,	64608,	64800,
		64984,	65173,	65366,	65565,	65763,	65971,	66179,	66394,	66621,	66854,	67117
};
bool over_heat_cur_limit = false;
float time_decay_rate = 1.0f/10000;
uint32_t cur_max_pre = 0;
uint32_t bp_vals[3] = {150, 90, 60};
static void pms_est_cur_dischar_limit(est_data_t *_this){

	uint32_t num = 0;
	uint32_t res_BP =150;//resistance 1 BP
	uint32_t cur_max =0;
	uint32_t soc =0;
	uint32_t soc_old =0;
	uint32_t ocv =0;
	uint8_t max_temp = 0;
	const sm_bp_data_t *bp_data = NULL;
	for (int i= 0; i <SM_BP_NUMBER_DEFAULT; i++){

		bp_data = sm_sv_bp_get_data(_this->m_bpm, i);
		if (bp_data->m_state == BP_STATE_DISCHARGING){
			num++;
			soc_old = soc;
			soc += (uint32_t)bp_data->m_soc;
			max_temp = (bp_data->m_temps[0] > max_temp) ?
							bp_data->m_temps[0] : max_temp;
			if ( soc < 1 ) soc = 1;
			if ((soc > soc_old) && (soc_old != 0) ) soc = soc_old;
		}
	}
	soc = CLAMP(soc,1,99);
	ocv = OCV_table[soc];
	res_BP = (num >= 1 && num <= 3) ? bp_vals[num - 1] : 0;
	if (res_BP > 0)
	    cur_max = 1000*(ocv-53000)/res_BP;;

	switch (num) {
	case 0:

		if (sm_bp_get_active_retain_numbs(_this->m_bp_reatain) > 0) {

			cur_max =(uint32_t)(3000
					+ sm_bp_retain_get_elapsed_ratio(_this->m_bp_reatain)
							* (float)cur_max_pre);
		} else {

			cur_max = 3000;
		}
		break;
	case 1:
		cur_max = (soc <= 20) ? cur_max - 2000 : cur_max;
		cur_max = CLAMP(cur_max,5000,35000);
		/* active overheat cur limit level 1*/
		if (max_temp > 55 && (cur_max > 20000)) {
			cur_max = 20000;
			over_heat_cur_limit = true;
		}
		/* hyteresis overheat cur limit*/
		if ((over_heat_cur_limit == true)
				&& (max_temp > 54)
				&& (cur_max > 20000)) {
			cur_max = 20000;
		} else {
			over_heat_cur_limit = false;
		}
		cur_max_pre =  _this->m_cur_dischar_limit;
		break;
	case 2:
		if (soc <= 20) {cur_max = cur_max - 2000;}
		cur_max = CLAMP(cur_max,15000,45000);
		cur_max_pre =  _this->m_cur_dischar_limit;
		break;
	case 3:
		if (soc <= 20) {cur_max = cur_max - 2000;}

		cur_max = CLAMP(cur_max,15000,65000);
		cur_max_pre =  _this->m_cur_dischar_limit;
		break;
	default:
		break;
	}

	 _this->m_cur_dischar_limit =  cur_max;
}

static void pms_est_cur_distance(est_data_t *_this){

    int32_t power_unused = 0;
	int32_t power_km_f = 28;
	int32_t distance_t;
	const sm_bp_data_t *bp_data = NULL;
	sm_mc_data_t*mc_data = sm_mc_get_data(_this->mc_module);
	sm_pmu_data_t*pmu_data = sm_pmu_get_data(_this->pmu_module);
	if (mc_data->m_cur_mode == EV_ECO_MODE_1
			|| mc_data->m_cur_mode == EV_ECO_MODE_2) {
		power_km_f = 22;
	} else {
		power_km_f = 28;
	}
	if (pmu_data->m_parking == 1) {
		power_km_f = 22;
	}
	if (power_km_f < 1)
		power_km_f = 1;

	int32_t soc_total_unused = 0;

	for (uint8_t i = 0; i < SM_BP_NUMBER_DEFAULT; i++) {
		bp_data = sm_sv_bp_get_data(_this->m_bpm, i);
		if (sm_sv_bp_is_connected(_this->m_bpm, i)) {
			soc_total_unused += bp_data->m_soc;
		}
	}

	if ( soc_total_unused < 5 ){// soc x10
		distance_t = 0;
	}
	else/* estimate distance = %SoC*sum_power/power_per_km )(km) */
	{
		power_unused = (soc_total_unused - 5)*11;// 10 is the power of BP divided by 100
		distance_t = (int32_t)(power_unused/power_km_f);// power_unused x 10
	}
	distance_t = (distance_t > 150) ? 150 : (distance_t < 0) ? 0 : distance_t;
	_this->m_distance = (uint16_t)distance_t;
}
static void pms_est_update_data(est_data_t *_this){

    sm_pmu_data_t*pmu_data = sm_pmu_get_data(_this->pmu_module);

    pmu_data->m_energy_in           = _this->m_elec_energycharge;
    pmu_data->m_energy_out          = _this->m_elec_energydischarge;
    pmu_data->m_range               = _this->m_distance;
    pmu_data->m_discharge_cur_lim   = _this->m_cur_dischar_limit;
}
int32_t pms_est_process(est_data_t *_this) {

	if(!_this) return -1;
	pms_est_energy(_this);
    pms_est_cur_dischar_limit(_this);
    pms_est_cur_distance(_this);
    pms_est_update_data(_this);
	return 0;
}
