"""
CSCI4-577 Comp Modeling and Simulation

Term Project

Scenario:

	The Montana Tech Biology Department wishes to put remote soil sensors in the Pintler Moutnains to record soil temperatures and email them back, via a
satellite uplink. Every hour, the system wakes up from sleep mode and takes a reading, writes the reading to a text file on an SD card and checks if it's time to
send a message. Should the message need to be sent, the microcontroller wakes up the satellite uplink and attempts to send an email.

Questions to Answer:
	How long will the system be able to operate?
	How often will the message get sent?
	Which power supply will run out first?

Jamison D. Ehlers, Undergraduate Student
Electrical Engineering, Montana Technological University
jehlers@mtech.edu
"""

##
# Import Required Packages
import random	# pseudo-random generator
import simpy	# DEDS package

##
# Manifest constants for simulation. All currents are in terms of milliamps
RANDOM_SEED = 42						# initial seed for random generator

CELL1_POWER = 19000						# 1-cell is rated for 19000 mAh (19Ah)
CELL2_POWER = CELL1_POWER * 2					# 2-cell is two 1-cell batteries in series

snowAccum = 0							# initial snow accumulation in feet

# Sleep Power Consumptions
sleepPowerRS = 0.000020						# sleep power consumption of microcontroller (20uA)
sleepPowerRegulation = 0.00001*4 				# estimated quiescent current of voltage regulators
msgSent = 0

# Awake Power Consumption
awakePowerRS = 5						# estimated power consumption of uC in awake mode
satPower = 400							# estimated power consumption to send a message (400mA)
sensorPower = 0.00245						# estimated power consumption of soil sensors
sdPower = 30							# estimated power consumption to write to SD card
regPower = 0.00018*4						# estimated power consumption of voltage LDOs

awakePower = sensorPower + sdPower + regPower    		# total estimated power consumption when taking a soil reading
sendpower = awakePower + satPower				# total estimated power consumption when taking a soil reading, then sending satellite message

sleepPower = sleepPowerRegulation
satTime = 7*24						# send message once every month
sensorTime = 1							# take a reading every day

##
# Accumulation variables
SIM_TIME = 26280						# how long the system can operate (couldn't figure out how to run simulation implicitly, so initially us 3 years: 26280 hours)

##
# Class Definitions

class	OneCell(object):
	global CELL1_POWER

	"""
	Single battery cell for Microcontroller power, only powers RocktScream
	"""
	def __init__(self,env,charge):
		self.env = env
		self.capacity = simpy.Container(env,capacity = charge, init = charge)

	def getCharge(self):
		return self.capacity.level


class	TwoCell(object):
	"""
	Two battery cells in series for RockBlock power, voltage regulation power, communication power and SD write power
	"""
	global CELL2_POWER

	def __init__(self,env,charge):
		self.env = env
		self.capacity = simpy.Container(env,capacity = charge, init = charge)

	def getCharge(self):
		return self.capacity.level

def RockBlock(env,twocell):
	"""
	Satelite modem sending messages via email
	"""
	global msgSent
	global snowAccum
	while True:
		yield env.timeout(satTime)
		if snowAccum < 3:
			yield twocell.capacity.get(satPower)
			msgSent += 1

def RocketScreamOn(env,onecell):
	while True:
		yield env.timeout(sensorTime)
		yield onecell.capacity.get(awakePowerRS)

def RocketScreamSleep(env,onecell):
	while True:
		yield env.timeout(.1)
		yield onecell.capacity.get(sleepPowerRS)

def ElectronicsOn(env,twocell):
	while True:
		yield env.timeout(sensorTime)
		yield twocell.capacity.get(awakePower)

def ElectronicsSleep(env,twocell):
	while True:
		yield env.timeout(.1)
		yield twocell.capacity.get(sleepPower)

class snowLevel(object):
	"""
	Creates and manages current snow level
	"""
	global snowAccum

	def __init__(self,env):
		self.env = env
		self.snowLevel = simpy.Container(env,capacity = 20, init = 0)
		self.monitor = env.process(self.snowing())

	def snowing(self):
		while True:
			yield env.timeout(24)

			snowMelt = random.random()
			snowFall = random.random()

			if(self.snowLevel.level > 0 and snowMelt < self.snowLevel.level and snowMelt > 0):
				yield self.snowLevel.get(snowMelt)

			if(snowFall > 0 and self.snowLevel.level < 10):
				if self.snowLevel.level > 3:
					yield self.snowLevel.put(snowFall*0.5)

			snowAccum = self.snowLevel.level


def setup(env,cell1Power,cell2Power):
	"""
	Creates any required objects from classes
	Accumulate Data
	"""
	env.onecell = OneCell(env,cell1Power)
	env.twocell = TwoCell(env,cell2Power)
	env.snowLevel = snowLevel(env)
	env.rockblock = [env.process(RockBlock(env,env.twocell))]
	env.rocketscreamon = [env.process(RocketScreamOn(env,env.onecell))]
	env.rocketscreamoff = [env.process(RocketScreamSleep(env,env.onecell))]
	env.electronicson = [env.process(ElectronicsOn(env,env.twocell))]
	env.electronicsoff = [env.process(ElectronicsSleep(env,env.twocell))]



##
# Setup and start simulation
print('HAST Snowfall and Life Expectancy simulation')
# Seed the random number generator
random.seed(RANDOM_SEED)
# Create an environment and start the setup process
env = simpy.Environment()
setup(env,CELL1_POWER,CELL2_POWER)
env.run(until = SIM_TIME)
print('Results:')
print('Time simulation ran: %d hours' % SIM_TIME)
print('Charge left in double cell: %.2f mAh' % env.twocell.getCharge())
print('Charge left in single cell: %.2f mAh' % env.onecell.getCharge())
print('Number of messages sent: %d' % msgSent)
