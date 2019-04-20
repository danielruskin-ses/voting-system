/* gpio.h */

#ifndef GPIO_H_
#define GPIO_H_


class GPIO
{
public:
	GPIO() {}
	~GPIO() {}

	bool getInput(int pin) const;
	void setOutput(int pin, bool state) const;
};


#endif /* GPIO_H_ */
