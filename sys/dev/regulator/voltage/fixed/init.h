#ifndef dev_regulator_voltage_fixed_init_h
#define dev_regulator_voltage_fixed_init_h

/*
 * Driver for fixed voltage regulator
 */

#ifdef __cplusplus
extern "C" {
#endif

struct regulator_voltage_fixed_desc {
	const char *name;
	float voltage;
};

void regulator_voltage_fixed_init(const struct regulator_voltage_fixed_desc *);

#ifdef __cplusplus
}
#endif

#endif
