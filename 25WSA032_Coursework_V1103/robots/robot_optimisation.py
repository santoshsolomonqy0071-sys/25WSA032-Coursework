"""
robot ecosystem operation code

This module creates a test ecosystem and runs it for a specified duration, 
demonstrating the use of the ecosystem factory and deliverable creation functions
It simulates the operation of delivery bots, including charging and delivering pizzas, while providing options for debugging and message display.
"""

from robots.ecosystem.factory import ecofactory

# Duration is set to two weeks for development and rapid testing. Set to 52 weeks for your final tests.

import matplotlib.pyplot as plt

min_battery = {
    'Drone': 0.15,
    'Droid': 0.20,
    'Robot': 0.25
}

def hypotenuse(a, b):
  
  return ((a[0]-b[0])**2 + (a[1]-b[1])**2)**0.5


def nearest_charger(bot, chargers):
  return min(chargers, key = lambda charger: hypotenuse(bot.coordinates, charger.coordinates))



def should_charge(bot, chargers):
  nearest = nearest_charger(bot, chargers)
  if nearest is None:
    return False
  minimum_battery = min_battery[bot.kind]
  distance = hypotenuse(bot.coordinates, nearest.coordinates)
  return bot.soc / bot.max_soc < minimum_battery and bot.station is None and distance < 20



opportunistic_charging_distance = 10
opportinistic_requirement = 0.4

def opportunistic_charge(bot, chargers):
  
  if bot.station is not None:
    return False
  if bot.soc / bot.max_soc >= opportinistic_requirement:
    return False
  chargers_in_range = [charger for charger in chargers if hypotenuse(bot.coordinates, charger.coordinates) < opportunistic_charging_distance]
  if chargers_in_range:
    target_charger = min(chargers_in_range, key = lambda charger: hypotenuse(bot.coordinates, charger.coordinates))
    bot.charge(target_charger)
    return True 
  return False



if __name__ == "__main__":
    plt.close('all')  # optional: cleans up leftovers from prior runs
    plt.ion()         # interactive mode ON (non-blocking windows)

# Create and configure the ecosystem using the factory function. 
# Study the factory function code to understand how the ecosystem is being created 
# and configured. Adjust the parameters as needed for your testing and development.  
es = ecofactory(robots = 3, droids = 3, drones = 3, chargers = [55,20], pizzas = 9)



charger = es.chargers()[0]
es.display(show = 1, pause = 10)                                                # show = 0 will turn off the display and speed up the run. Set to 1 for development and debugging, set to 0 for final runs. Note that when show = 0, you will not see the ecosystem or any messages, so it is wise to turn on messages (es.messages_on = True) when show = 0 for development and debugging. 
es.debug = False                                                                # this will directly display damage and warning messages. Note show needs to be zero  (show = 0)
es.messages_on = False                                                          # over 52 weeks it is wise to turn messages off as there are too many. But when researching turn on for shorter runs
es.duration = "1 week"                                                          # We are aiming to run for a year with minimum or no bot breakages

home = [40,20, 0]                                                               # Place to which bots will return when idle and from which they will start. This is also the location of the charger in this example, but it doesn't have to be. You can change this and the charger location to test the bots' ability to navigate around the ecosystem.
                             

while es.active:

  for bot in es.bots():
    if should_charge(bot, es.chargers()):
      bot.charge(nearest_charger(bot, es.chargers()))                                                       # initiate charging.
    elif opportunistic_charge(bot, es.chargers()):
      opportunistic_charge(bot, es.chargers())



      
    #create_deliverables(es)                                                     # Use the create deliverables function to maintain a stock of ready pizzas

                                                
    if bot.activity == 'idle':                                                  # if bot is idle, contract to deliver a ready pizza.
      for pizza in es.deliverables():
        if pizza.status == 'ready':
          bot.deliver(pizza)                                                    # ensure we do not contract to deliver a pizza already contracted by another bot
          break
      if not bot.destination and bot.coordinates != home:
        bot.target_destination = home                                           # if we get here, we've gone through the list of pizzas and none was ready
    if bot.target_destination:bot.move()                                        # move whilst we have a destination. At the end of delivery, the bot status will be set to idle

  es.update()                                                                   # update when all bots have been processed and moved

