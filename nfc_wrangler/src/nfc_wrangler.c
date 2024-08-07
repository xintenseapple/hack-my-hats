#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <Python.h>

#include "tophat/api/client.h"
#include "tophat/devices/nfc_reader.h"
#include "tophat/devices/neopixel.h"

#define NFC_DEVICE_NAME "nfc_reader"
#define NEOPIXEL_DEVICE_NAME "neopixels"

static PyObject *tophat_client;
static bool stop = false;

void wrangle_data(char *, char *);
void handle_request();
void bootsnake();
void yeehaw();

static char gNFC_data[32];
static long tarnation;
static char a_hacker;
static char is_dev;

void interrupt_handler(int signum) {
    printf("Received interrupt, stopping...\n");
    fflush(stdout);
    stop = true;
}

int main(int argc, char *argv[])
{
    Py_Initialize();
    PyGILState_STATE gil_state = PyGILState_Ensure();
    signal(SIGINT, interrupt_handler);

    tophat_client = get_client(DEFAULT_SOCKET_PATH);

    if (tophat_client == NULL) {
        fprintf(stderr, "Failed to create tophat client!\n");
        fflush(stderr);
        return -1;
    }

    printf("Wrangling has begun...\n");
    fflush(stdout);
    while (!stop) { // This might take a while...
        handle_request();
    }

    Py_DECREF(tophat_client);
    PyGILState_Release(gil_state);
    Py_Finalize();
    printf("Wrangling completed!\n");
    fflush(stdout);
    return 0;
}

void bootsnake()
{
    printf("There's a snake in my boot!\n");
    fflush(stdout);
}

void yeehaw()
{
    printf("Yeehaw!\n");
    fflush(stdout);
    // system("/home/hat/yeehaw.sh");
}

void check_lights(char* flag_token)
{
    //Use the rand_key generator NFC tag
    bool check = true;
    for(int i=0; i<10; i++)
    {
        printf("%#hhx == %#hhx\n", gNFC_data[i], flag_token[i]);
        fflush(stdout);
        if (gNFC_data[i] != flag_token[i] )
            check = false;
    }
    if (check) {
        printf("Passed check!\n");
        fflush(stdout);

        PyObject *color_pyobj = create_color(255, 255, 255);
        PyObject *command_pyobj = create_blink_command(5, color_pyobj, 2);

        if (command_pyobj == NULL) {
            fprintf(stderr, "Failed to create command!\n");
            fflush(stderr);
        } else {
            Py_DECREF(send_command(tophat_client, NEOPIXEL_DEVICE_NAME, command_pyobj));
        }

        Py_DECREF(command_pyobj);
        Py_DECREF(color_pyobj);
    }
}

void run_lights() {
    /*
    Rundown of the light commands:
    solid = Solid color
    blink = Blink one color
    pulse = Pulse one color
    rainbow = Rainbow pulse across all LEDs
    rainbow_wave = Rainbow cycle across each LED
    */
    if (is_dev && !a_hacker) {
        // Partner, if you got this far, and like hackin this much
        // I reckon you might give us a holler at hackmyhat@proton.me
        // If'n yer lookin fer a job that is.
        // Also check us out at https://nightwing.us/careers
        printf("Well I'll be darned, this here hat's been hacked!\n");
        fflush(stdout);

        PyObject *color_pyobj = create_color(255, 255, 255);
        PyObject *command_pyobj;
        if (strcmp(gNFC_data, "solid") == 0) {
            command_pyobj = create_solid_color_command(color_pyobj);
        } else if (strcmp(gNFC_data, "blink") == 0) {
            command_pyobj = create_blink_command(5, color_pyobj, 2);
        } else if (strcmp(gNFC_data, "pulse") == 0) {
            command_pyobj = create_pulse_command(5, color_pyobj, 2, 0);
        } else if (strcmp(gNFC_data, "rainbow") == 0) {
            command_pyobj = create_rainbow_command(5, 10);
        } else if (strcmp(gNFC_data, "rainbow_wave") == 0) {
            command_pyobj = create_rainbow_command(5, 10);
        } else {
            fprintf(stderr, "Received invalid command `%s`!\n", gNFC_data);
            fflush(stderr);
            return;
        }

        if (command_pyobj == NULL) {
            fprintf(stderr, "Failed to create command!\n");
            fflush(stderr);
        } else {
            Py_DECREF(send_command(tophat_client, NEOPIXEL_DEVICE_NAME, command_pyobj));
        }

        Py_DECREF(command_pyobj);
        Py_DECREF(color_pyobj);
    }
}

void format_NFC_data(char* buffer) {
    tarnation = &bootsnake;
    for(size_t i = 0; i < strlen(buffer); i++) {
        char c = buffer[i];
        if (c == 33) {
            c = 0;
        }
        gNFC_data[i] = c;
    }
    printf("tarnation: %#lx\nis_dev: %#hhx\na_hacker: %#hhx\n", tarnation, is_dev, a_hacker);
    fflush(stdout);
}

void handle_token(char* flag_token) {
    // Admin function to test the LEDS, should get removed for production
    check_lights(flag_token);
    run_lights();
    // Now where in tarnation did I put that third flag?
    if (tarnation == bootsnake || tarnation == 0x796D6E69746F6F62) {
        if (tarnation == 0x796D6E69746F6F62) {
            tarnation = &yeehaw;
        }
        ((void (*)(void))tarnation)();
    }
}

void generate_flag_token(char *flag_buf) {
    flag_buf[11] = 0; // Make sure we null terminate!
    // Utilize random memory to make a random string!
    for(int i=0; i<10; i++) {
        flag_buf[i] = flag_buf[i+20];
    }
}

void wrangle_data(char *nfc_card_data, char *flag_buf) {
    printf("Received nfc_card_data '%s' of size %#lx\n", nfc_card_data, strlen(nfc_card_data));
    fflush(stdout);
    generate_flag_token(flag_buf);
    format_NFC_data(nfc_card_data);
    handle_token(flag_buf);
}

void handle_request() {
    char nfc_card_data[43] = {0};
    char flag_buf[11] = {0};

    printf("Awaiting connection...\n");
    fflush(stdout);

    PyObject *command_pyobj = create_read_data_command(1.0);
    if (command_pyobj == NULL) {
        fprintf(stderr, "Failed to create command!\n");
        fflush(stderr);
        return;
    }

    PyObject *result_pyobj = send_command(tophat_client, NFC_DEVICE_NAME, command_pyobj);
    if (result_pyobj == NULL) {
        fprintf(stderr, "Failed to send command!\n");
        fflush(stderr);
        return;
    }

    Py_DECREF(command_pyobj);

    ssize_t data_len = PyByteArray_Size(result_pyobj);
    if (data_len == 0) {
        Py_DECREF(result_pyobj);
        return;
    }

    printf("Handling new connection...\n");
    fflush(stdout);

    // No don't forget, raw_nfc_data ain't include the first 4 blocks of any decent NFC data!
    char *raw_nfc_data = PyByteArray_AsString(result_pyobj);
    for (unsigned i = 0; i < data_len && i < sizeof(nfc_card_data) - 1; i++) {
        char current_char = raw_nfc_data[i];
        if (current_char < 32 || current_char > 126) {
            break;
        } else {
            nfc_card_data[i] = current_char;
        }
    }

    Py_DECREF(result_pyobj);

    wrangle_data(nfc_card_data, flag_buf);

    printf("Done handling\n");
    fflush(stdout);
}