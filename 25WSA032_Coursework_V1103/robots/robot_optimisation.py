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

def nearest_pizza(bot, pizzas):
  available_pizzas = [pizza for pizza in pizzas if pizza.status == 'ready' and pizza.weight <= bot.max_payload]
  if not available_pizzas:
        return None
  return min(available_pizzas, key = lambda pizza: hypotenuse(bot.coordinates, pizza.coordinates))


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



def baseline_test():
  plt.close('all')
  plt.ion()
  es = ecofactory (robots = 3, droids = 3, drones = 3, chargers= [55,20], pizzas = 9)
  charger = es.chargers()[0] 
  es.display(show = 1, pause = 10)                                               
  es.debug = False                                                               
  es.messages_on = False                                                          
  es.duration = "1 week"
  home = [40,20, 0]
  
  while es.active:

    for bot in es.bots():

      if bot.soc / bot.max_soc < 0.20 and bot.station is None:        
        bot.charge(charger)                                                       
      if bot.activity == 'idle':                                                  
        for pizza in es.deliverables():
          if pizza.status == 'ready':
            bot.deliver(pizza)                                                    
            break
        if not bot.destination and bot.coordinates != home:
          bot.target_destination = home                                           
      if bot.target_destination:bot.move()                                        

    es.update()
  return es





# Create and configure the ecosystem using the factory function. 
# Study the factory function code to understand how the ecosystem is being created 
# and configured. Adjust the parameters as needed for your testing and development.  
#es = ecofactory(robots = 3, droids = 3, drones = 3, chargers = [55,20], pizzas = 9)

def optimisation_test():

  plt.close('all')  # optional: cleans up leftovers from prior runs
  plt.ion()   

  es = ecofactory(robots = 3, droids = 3, drones = 3, chargers=([20,10], [60, 20]), pizzas = 9, max_weight = 50)


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
        pass




      #create_deliverables(es)                                                     # Use the create deliverables function to maintain a stock of ready pizzas

                                      

      if bot.activity == 'idle':
        pizza = nearest_pizza(bot, es.deliverables())
        if pizza:
              bot.deliver(pizza)
        elif not bot.destination and bot.coordinates != home:
              bot.target_destination = home
      
      if bot.target_destination:
        bot.move()                                        # move whilst we have a destination. At the end of delivery, the bot status will be set to idle



    es.update()                                                                   # update when all bots have been processed and moved
  return es


def print_results(baseline_es, optimisation_es):

  baseline_es.tabulate('name', 'kind', 'units_delivered', 'weight_delivered', 'distance', 'energy', 'status', kind_class = 'Bot')
  
  optimisation_es.tabulate('name', 'kind', 'units_delivered', 'weight_delivered', 'distance', 'energy', 'status', kind_class = 'Bot')

  baseline_units = sum(bot.units_delivered for bot in baseline_es.bots())
  baseline_weight = sum(bot.weight_delivered for bot in baseline_es.bots())
  baseline_distance = sum(bot.distance for bot in baseline_es.bots())
  baseline_energy = sum(bot.energy for bot in baseline_es.bots())
  baseline_broken = sum(1 for bot in baseline_es.bots() if bot.status == 'broken')


  optimisation_units = sum(bot.units_delivered for bot in optimisation_es.bots())
  optimisation_weight = sum(bot.weight_delivered for bot in optimisation_es.bots())
  optimisation_distance = sum(bot.distance for bot in optimisation_es.bots())
  optimisation_energy = sum(bot.energy for bot in optimisation_es.bots())
  optimisation_broken = sum(1 for bot in optimisation_es.bots() if bot.status == 'broken')


  print(f"Pizzas delivered - Baseline: {baseline_units}, Optimisation: {optimisation_units}, Improvement: {optimisation_units - baseline_units}")
  print(f"Weight delivered - Baseline: {baseline_weight}, Optimisation: {optimisation_weight}, Improvement: {optimisation_weight - baseline_weight}")
  print(f"Distance traveled - Baseline: {baseline_distance}, Optimisation: {optimisation_distance}, Improvement: {optimisation_distance - baseline_distance}")
  print(f"Energy consumed - Baseline: {baseline_energy}, Optimisation: {optimisation_energy}, Improvement: {optimisation_energy - baseline_energy}")
  print(f"Broken bots - Baseline: {baseline_broken}, Optimisation: {optimisation_broken}, Improvement: {optimisation_broken - baseline_broken}")
  print(f"Efficiency - Baseline: {baseline_weight/baseline_energy if baseline_energy > 0 else 0}, Optimisation: {optimisation_weight/optimisation_energy if optimisation_energy > 0 else 0}, Improvement: {(optimisation_weight/optimisation_energy if optimisation_energy > 0 else 0) - (baseline_weight/baseline_energy if baseline_energy > 0 else 0)}")




if __name__ == "__main__":
  baseline_es = baseline_test()
  optimisation_es = optimisation_test()
  print_results(baseline_es, optimisation_es)

  
  