#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
    EV Charging Simulator

    This program simulates a number of cars N attempting to charge on a number of chargers M, where N < M.

    Each car needs Y minutes of charging time.

    Each car can make reservations on up to M chargers. As time passes and chargers become free, reservations are fulfilled.

    Once a car is charging on any charger, its reservations on any other chargers are canceled. Once a car has finished charging,
    the charger is freed and the car is marked as charged.

    The simulation ends when all cars have been charged.

    TODO:
    - [ ] add formatter, linter, valgrind, and so on.
    - [ ] reorg this file for readability.
    - [ ] improve simulation by making reservations once clock has begun, simulating people arriving later in the day to reserve.
    - [ ] randomize which chargers we pick. current implementation loads up the first few chargers and leaves the last ones idle.

*/

#define NUM_CHARGERS 3
#define CLOCK_TICK_MINUTES 5 // every clock tick adds this many minutes to the clock

static inline int imax(int a, int b) { return a > b ? a : b; }

typedef struct Car
{
    char *owner;
    int charge_minutes;
    int charged;
} Car;

struct Reservation;
typedef struct Reservation {
    Car *car;
    struct Reservation *next;
} Reservation;

typedef struct Charger {
    int id;
    Car *charging_car;
    int charging_minutes;
    Reservation *reservations;
} Charger;


// all_charged returns 1 if all cars are charged, otherwise it returns 0
int all_charged(int num_cars, Car *cars) {
    for (int c = 0; c < num_cars; ++c) {
        if (!cars[c].charged) {
            return 0;
        }
    }
    return 1;
}

// is_reserved_for return 1 if charger has a reservation for car
int is_reserved_for(const Car *car, const Charger *charger) {
    for (Reservation *r = charger->reservations; r != 0; r = r->next) {
        if (r->car == car) {
            return 1;
        }
    }

    return 0;
}

// cancel_reservation_for removes any reservation for car from charger and prints the cancelation to stdout
void cancel_reservation_for(const Car *car, Charger *charger) {
    if (charger->reservations == NULL) return;

    Reservation **pr = &charger->reservations;
    while (*pr) {
        Reservation *r = *pr;
        if (r->car == car) {
            *pr = r->next;
            free(r);
            printf("❌ canceled reservation for %s on %d\n", car->owner, charger->id);
            return;
        }
        pr = &r->next;
    }
}

// make_reservation allocates memory for and returns a pointer to a new Reservation for car
Reservation *make_reservation(Car *car) {
    Reservation *r = calloc(1, sizeof(Reservation));
    r->car = car;
    return r;
}

// reserve adds car to charger's list of reservations and prints the reservation to stdout
void reserve(Car *car, Charger *charger) {
    if (charger->reservations == NULL) {
        charger->reservations = make_reservation(car);
    } else {
        for (Reservation *r = charger->reservations; ; r = r->next) {
            if (r->next == NULL) {
                r->next = make_reservation(car);
                break;
            }
        }
    }

    printf("👉 reserved charger %d for %s\n", charger->id, car->owner);
}

// make_reservations distributes charger reservations for cars. Each car may reserve up to num_chargers chargers.
void make_reservations(int num_cars, Car *cars, int num_chargers, Charger *chargers) {
    for (int car = 0; car < num_cars; ++car) {
        // reserve a random number of chargers, or 2 chargers, whichever is greater
        int num_reservations = imax(2, rand() % (num_chargers + 1));
        printf("📞 making %d reservations for %s\n", num_reservations, cars[car].owner);
        assert(num_reservations <= num_chargers);

        for (int charger = 0; charger < num_reservations; ++charger) {
            // TODO: randomize which chargers we pick. this loads up the first few chargers and leaves the last ones idle.
            reserve(&cars[car], &chargers[charger]);
        }
    }
}

// fulfill_next_reservaion promotes charger's next reservation to charging and prints
void fulfill_next_reservaion(Charger *charger) {
    assert(charger->reservations != NULL);

    Reservation *res = charger->reservations;
    charger->reservations = res->next;

    charger->charging_car = res->car;
    charger->charging_minutes = 0;

    printf("⚡️ %s's car is now charging on %d\n", charger->charging_car->owner, charger->id);
    free(res);
}

// free_charger marks charger as free for use
void free_charger(Charger *charger)
{
    charger->charging_car = NULL;
    charger->charging_minutes = 0;
}

// validate cars asserts that all cars have been initialized according to our assumptions
void validate_cars(int num_cars, Car *cars) {
    for (int car = 0; car < num_cars; ++car) {
        assert(cars[car].owner != NULL);
        assert(strlen(cars[car].owner) > 0);
        assert(cars[car].charge_minutes % 5 == 0);
    }
}

int main(void) {
    srand(time(NULL));

    Car cars[] = {
        {"Alice", 30, 0},
        {"Bob", 45, 0},
        {"Charlie", 20, 0},
        {"Delroy", 60, 0},
        {"Egon", 30, 0},
        {"Fairuza", 60, 0},
        {"Galadriel", 120, 0},
        {"Horace", 120, 0},
    };
    int num_cars = sizeof(cars) / sizeof(cars[0]);
    validate_cars(num_cars, cars);
    assert(num_cars > NUM_CHARGERS);

    Charger chargers[NUM_CHARGERS];
    for (int i = 0; i < NUM_CHARGERS; ++i) {
        chargers[i].id = i;
        chargers[i].reservations = NULL;
        free_charger(&chargers[i]);
    }

    make_reservations(num_cars, cars, NUM_CHARGERS, chargers);

    while (!all_charged(num_cars, cars)) {
        for (int c = 0; c < NUM_CHARGERS; ++c) {
            Charger *charger = &chargers[c];
            if (charger->charging_car == 0 && charger->reservations != 0) {
                fulfill_next_reservaion(charger);
                for (int c1 = 0; c1 < NUM_CHARGERS; ++c1) {
                    if (c1 != c) {
                        cancel_reservation_for(charger->charging_car, &chargers[c1]);
                    }
                }
            } else if (charger->charging_car != NULL) {
                charger->charging_minutes += CLOCK_TICK_MINUTES;
                if (charger->charging_minutes == charger->charging_car->charge_minutes) {
                    charger->charging_car->charged = 1;
                    free_charger(charger);
                }
            }
        }
    }

    printf("🏆 All cars charged!\n");

    return 0;
}
