/*
 * sm_ec200_ssl.c
 *
 *  Created on: Nov 25, 2024
 *      Author: vuonglk
 */
#include <stdio.h>
#include <string.h>
#include "sm_ec200_ssl.h"

#define SSL_UPLOAD_TIMEOUT_MS 5000


const char *CA_CERT =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDpTCCAo2gAwIBAgIUYDPbEvFG0AC6XPlAEE7wQMrmUQgwDQYJKoZIhvcNAQEL\n"
        "BQAwYTELMAkGA1UEBhMCVk4xCzAJBgNVBAgMAkhOMQswCQYDVQQHDAJITjEOMAwG\n"
        "A1UECgwFU2VsZXgxDDAKBgNVBAsMA0lPVDEaMBgGA1UEAwwRc2VsZXgtdWF0LXJv\n"
        "b3QtY2EwIBcNMjQwNDAyMTEwNTU4WhgPMjEyNDAzMDkxMTA1NThaMGExCzAJBgNV\n"
        "BAYTAlZOMQswCQYDVQQIDAJITjELMAkGA1UEBwwCSE4xDjAMBgNVBAoMBVNlbGV4\n"
        "MQwwCgYDVQQLDANJT1QxGjAYBgNVBAMMEXNlbGV4LXVhdC1yb290LWNhMIIBIjAN\n"
        "BgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAycRzmRvcq/nFGwg6tAyJf3ETQX7a\n"
        "OPBYS2TUEm3ZrLCjfnie3GtDYFWL9y9/QbZWknuRojB6fU263UO4cAqDMNxNGnoX\n"
        "CVod7Qdz+0GVOKGF+zF4hab/XZ4SrQx7G9oILBEyOaJvLNhvZ6AfGNgXRxSXVE0C\n"
        "kpyTb3Th+dF/buTNDgl0zMyoQkOPrrQs6ECUMZK6+LEY+Ncvyg6OFKQMU44w7IGH\n"
        "irBVAMeoC+8Eht+9fEmthd44GDSTpBND929zrJLkXFHeRiClbuLrM2k1tSu8j0gM\n"
        "qNTnlKy7A5x6XPpvZ+vSCGFXLII3x7tN09uubpc/jRmEygiFn7fUVAreWQIDAQAB\n"
        "o1MwUTAdBgNVHQ4EFgQUn3vW6GcgicPSJ/otwIY4fS0B+ywwHwYDVR0jBBgwFoAU\n"
        "n3vW6GcgicPSJ/otwIY4fS0B+ywwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0B\n"
        "AQsFAAOCAQEAiAvU7ICqg1DlZM27dmgF/rhA2WWOTGnQCZfeTPqxNOChJ4CHxFYT\n"
        "jzStOrO/d6xbLJRET8zx17JKDaX2gs0QaFsseIjbWn2QdCbjWXZryqfQbt2+dGT+\n"
        "xR78lIrej1Tt5KRMWoae3EOEpobR0O8mq8Px/OC14PA1Oj5QMPTQiailSy668Z1b\n"
        "QW80WDsxStD19XrIjQvPoIscMBILQ2To6y7aieUJzBgTpbfFJu8hlRaf+UCJ+bvQ\n"
        "NLBxmfszz0sMPvWGHSCStBlPl9DrDQ3eSxTW6fvkdMjKAOP9XuSiS/PjkzEiCM6K\n"
        "3zCXH+GYBP2ZyZ4+T+vtCyfqaCm7i8rizw==\n"
        "-----END CERTIFICATE-----";


const char *CLIENT_CERT =
        "-----BEGIN CERTIFICATE-----\n"
        "MIID0DCCArigAwIBAgIIQpfAF4JXyIwwDQYJKoZIhvcNAQELBQAwYTELMAkGA1U\n"
        "EBhMCVk4xCzAJBgNVBAgMAkhOMQswCQYDVQQHDAJITjEOMAwGA1UECgwFU2VsZX\n"
        "gxDDAKBgNVBAsMA0lPVDEaMBgGA1UEAwwRc2VsZXgtdWF0LXJvb3QtY2EwHhcNM\n"
        "jQwNDAyMDY0MzQ3WhcNMzQwNDAyMDY0MzQ3WjAbMRkwFwYDVQQDDBBzb25sdF9x\n"
        "MnExOG54MWlsMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAslZnCN0\n"
        "57sTxES9eyF2F/4q3pO/YoASz+xTXXzQ3rmGXC16qoFhDWf2ZSBy0iGt4BoMUby\n"
        "FvhuLZLwI2gGoIilq1PBiR6KsKM10Ev7YUUFf1VfhxaYPYdkrZVPCt7QhB7cOwT\n"
        "t2ks4yDXxAgg5zr7GRx/z4LyYLWBoX8wIERqU7P+rlH/4nLOVUc8PH3I8o8wWJ6\n"
        "d7qE2y5TYpHNTcTbLi0S95BQ38qPwqsPDU4oYmIMON2PWQKRKHiTjIADlpoGWGz\n"
        "NOyW/Ef/eZeBL0OY6DRhJUeugJE9iOaUkB/fNA9Z4bRL8zGzf7UDaDoHr7kjg8j\n"
        "UeCdskoZ5dBlL7EG55pwIDAQABo4HRMIHOMAwGA1UdEwEB/wQCMAAwgZ4GA1UdI\n"
        "wSBljCBk4AUn3vW6GcgicPSJ/otwIY4fS0B+yyhZaRjMGExCzAJBgNVBAYTAlZO\n"
        "MQswCQYDVQQIDAJITjELMAkGA1UEBwwCSE4xDjAMBgNVBAoMBVNlbGV4MQwwCgY\n"
        "DVQQLDANJT1QxGjAYBgNVBAMMEXNlbGV4LXVhdC1yb290LWNhghRgM9sS8UbQAL\n"
        "pc+UAQTvBAyuZRCDAdBgNVHQ4EFgQUmQxkmEy7NntUtohuMI8r3EXH1W8wDQYJK\n"
        "oZIhvcNAQELBQADggEBAKTtMza+UCjSVu8BZ24dTmWvMqNs1HcOr52OO4vaHmem\n"
        "vcJh0EiaZNsNXbJhZdydnNda6XuLqVy3ZPnZgd/FVUatwdkX7jnYfqUatgiQwNX\n"
        "thm5PB2IKz5WFBumgQOI6vhlvnMUq3VXzc5RJl4h2U3Q8Bw+YfqdzeCIqs1Ryrh\n"
        "oajj6+pn7WgHBhq9qaXk8tkkHgpZzSPeoZKwvyJqvQ1hweMj6JorGEf6m/+ulK8\n"
        "NH0HZcLVMxdhqdl3KpNbir2hS432vczdRug4KCOQsj4Jpr4Pf+6UAFjc42GgPH/\n"
        "Dk44TRKc8fsbmpLFKa3I5ijHxqoJiLPfGIdEZ4BaJET9e34=\n"
        "-----END CERTIFICATE-----"
        ;


const char *CLIENT_KEY =
        "-----BEGIN PRIVATE KEY-----\n"
        "MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCyVmcI3TnuxPE\n"
        "RL17IXYX/irek79igBLP7FNdfNDeuYZcLXqqgWENZ/ZlIHLSIa3gGgxRvIW+G4t\n"
        "kvAjaAagiKWrU8GJHoqwozXQS/thRQV/VV+HFpg9h2StlU8K3tCEHtw7BO3aSzj\n"
        "INfECCDnOvsZHH/PgvJgtYGhfzAgRGpTs/6uUf/ics5VRzw8fcjyjzBYnp3uoTb\n"
        "LlNikc1NxNsuLRL3kFDfyo/Cqw8NTihiYgw43Y9ZApEoeJOMgAOWmgZYbM07Jb8\n"
        "R/95l4EvQ5joNGElR66AkT2I5pSQH980D1nhtEvzMbN/tQNoOgevuSODyNR4J2y\n"
        "Shnl0GUvsQbnmnAgMBAAECggEAFiQNAbg5j4/K4BfR/C1ETCAkQal3axJ6Z+NEA\n"
        "/FfMlqyXnp/041xuzP1elL6BO/WxHQ0rsFnb8ctMJ1lN9cyPT8avh3My6akixDy\n"
        "3HxWFE1/S4+fQPrZ/oG3bsi8xy9dxA+mDXyODK6WxtAhQUXTspMb5hUFcJy6RrA\n"
        "O27WKhErrhJt5N8GjOYnWRT71EcxjZzz5PIPIIYaesNNbdOvepm2RgKfo8199TN\n"
        "t9e/KoZy4qJAKiSsWy57CIvDV/49hPm6JLmNI7B5YuJuheb2Fr3A/IDuL2SH+AW\n"
        "s8dk2CP77gHeqn85bfCW5uLqFozNGsygQPdni1JrA4fDNJXePrWLQKBgQD2ksG/\n"
        "oSAYnsDhRx8jDL3yIbM1HQcZ54W5EsltG3PpgJyxHM3j47VMiC60br5NHMePJVt\n"
        "IHJw1X0aYmtbXxWLSQMu3iu94SAl6ivtFFBOS1Qki3Ub2pWAgcPrhovLryu6m2D\n"
        "EM2CxH/C08NmvhWyBf1TWk2GRyKkk/zgqlp7YMIwKBgQC5J9BVyNdxF4x7wFACr\n"
        "c4ZJNs5yr2Xbldfso9O9jIPivqmuGeKgQRMa6dOYdVlEeRCdHAxc13DdeX/xBQ8\n"
        "t8V3NHfgXs538KvYqMXuEAgeBj+hlNhZyQFygrEjPw3uaYhuj5kJBhZF7+pgMfs\n"
        "XYGlGEOODozzHOJXwz7fYdXACrQKBgE5PQbVoqNbo5H0np13sWA8kYNQHig/SFg\n"
        "g3TT2XsHKCZEFj0ogD87qArcSkP1AiScbOqHT4kSvqPHOPCZixmbcI2SpLt85No\n"
        "qUGqtK/k4/YlPQ7rqP61dK3zxYGOHxmkPDwTHO13Cpojr1G03BZDqkg/86A4zXW\n"
        "9DiNaZg+8XRdAoGAMcQYxCXyekw5Dz4Wg1HR9oIohnUO1nfq9ibZTADyXakeS9d\n"
        "+xQR+ckfxNSkooTzY3F/NlA0NI685vbAnPFastaidm+n/ls9ScS6T3LPkaqWFTt\n"
        "nh+OgFApFMKUwWY4FsEUN5f6pqLCwbE/X7JkJcqV96t3QVrvpvmsVKBd0y2RUCg\n"
        "YB7EljyIB9Iv2Nkcz+5RO9YD1ib403ueCeCFskP5aR1y7MFUn1Kr1SNK6vFF4qY\n"
        "7qLKml6QZY8h05f3QHBU2LttIhUilUkUbgwqyP9nfJHueNDjCeE7IPt6IX+87ZR\n"
        "zogzDH2MWad/tqZUAEPPMCbExba0DpNc+dyIa9jeoGgnY6A==\n"
        "-----END PRIVATE KEY-----";


int32_t sm_ec200_upload_file(sm_ec200_t *modem, const char *file_name, const char *data, uint32_t data_size) {
    sm_modem_t *p_modem = (sm_modem_t*) modem;
    char command[2048] = {0};

    snprintf(command, sizeof(command), "AT+QFLST=\"%s\"\r\n", file_name);
    if (sm_modem_send_cmd(p_modem, command, file_name, "ERROR\r\n", 5000) == 0) {
        snprintf(command, sizeof(command), "AT+QFDEL=\"%s\"\r\n", file_name);
        if (sm_modem_send_cmd(p_modem, command, "OK\r\n", "ERROR\r\n", 5000) != 0) {
            return -1;
        }
    }

    snprintf(command, sizeof(command), "AT+QFUPL=\"%s\",%u,100\r\n", file_name, data_size);
    if (sm_modem_send_cmd(p_modem, command, "CONNECT\r\n", "ERROR\r\n", 5000) != 0) {
        return -1;
    }

    if (sm_modem_send_cmd(p_modem, data, "\r\nOK\r\n", "ERROR\r\n", 5000) != 0) {
        return -1;
    }

    return 0;
}

int32_t sm_ec200_upload_certificate_authority(sm_ec200_t *modem, const char *ca_file_name) {
    return sm_ec200_upload_file(modem, ca_file_name, CA_CERT, strlen(CA_CERT));
}

int32_t sm_ec200_upload_client_certificate(sm_ec200_t *modem, const char *cc_file_name) {
    return sm_ec200_upload_file(modem, cc_file_name, CLIENT_CERT, strlen(CLIENT_CERT));
}

int32_t sm_ec200_upload_client_key(sm_ec200_t *modem, const char *ck_file_name) {
    return sm_ec200_upload_file(modem, ck_file_name, CLIENT_KEY, strlen(CLIENT_KEY));
}


int32_t sm_ec200_config_certificate_authority(sm_ec200_t *modem, const char *ca_file_name) {
    sm_modem_t *p_modem = (sm_modem_t*) modem;
    char command[128] = {0};
    sprintf(command, "AT+QSSLCFG=\"cacert\",2,\"%s\"\r\n", ca_file_name);
    return sm_modem_send_cmd(p_modem, command, "OK\r\n", "ERROR\r\n", 5000);
}

int32_t sm_ec200_config_client_certificate(sm_ec200_t *modem, const char *cc_file_name) {
    sm_modem_t *p_modem = (sm_modem_t*) modem;
    char command[128] = {0};
    sprintf(command, "AT+QSSLCFG=\"clientcert\",2,\"%s\"\r\n", cc_file_name);
    return sm_modem_send_cmd(p_modem, command, "OK\r\n", "ERROR\r\n", 5000);
}

int32_t sm_ec200_config_client_key(sm_ec200_t *modem,  const char *ck_file_name) {
    sm_modem_t *p_modem = (sm_modem_t*) modem;
    char command[128] = {0};
    sprintf(command, "AT+QSSLCFG=\"clientkey\",2,\"%s\"\r\n", ck_file_name);
    return sm_modem_send_cmd(p_modem, command, "OK\r\n", "ERROR\r\n", 5000);
}

int32_t sm_ec200_config_ssl_auth_mode(sm_ec200_t *modem) {
    sm_modem_t *p_modem = (sm_modem_t*) modem;
    char command[128] = {0};
    sprintf(command, "AT+QSSLCFG=\"seclevel\",2,2\r\n");
    return sm_modem_send_cmd(p_modem, command, "OK\r\n", "ERROR\r\n", 5000);
}

int32_t sm_ec200_config_ssl_version(sm_ec200_t *modem) {
    sm_modem_t *p_modem = (sm_modem_t*) modem;
    char command[128] = {0};
    sprintf(command, "AT+QSSLCFG=\"sslversion\",2,4\r\n");
    return sm_modem_send_cmd(p_modem, command, "OK\r\n", "ERROR\r\n", 5000);
}

int32_t sm_ec200_config_cipher_suite(sm_ec200_t *modem) {
    sm_modem_t *p_modem = (sm_modem_t*) modem;
    char command[128] = {0};
    sprintf(command, "AT+QSSLCFG=\"ciphersuite\",2,\"0xFFFF\"\r\n");
    return sm_modem_send_cmd(p_modem, command, "OK\r\n", "ERROR\r\n", 5000);
}

int32_t sm_ec200_ignore_local_time(sm_ec200_t *modem) {
    sm_modem_t *p_modem = (sm_modem_t*) modem;
    char command[128] = {0};
    sprintf(command, "AT+QSSLCFG=\"ignorelocaltime\",2,1\r\n");
    return sm_modem_send_cmd(p_modem, command, "OK\r\n", "ERROR\r\n", 5000);
}

int32_t sm_ec200_get_mem_space(sm_ec200_t *modem){
    sm_modem_t *p_modem = (sm_modem_t*)modem;
    char command[128] = {0};
    sprintf(command, "AT+QFLDS=\"UFS\"\r\n");
    return sm_modem_send_cmd(p_modem, command, "OK\r\n", "ERROR\r\n", 5000);
}

int32_t sm_ec200_get_list_files(sm_ec200_t *modem){
    sm_modem_t *p_modem = (sm_modem_t*)modem;
    char command[128] = {0};
    sprintf(command, "AT+QFLST=\"*\"\r\n");
    return sm_modem_send_cmd(p_modem, command, "OK\r\n", "ERROR\r\n", 5000);
}

int32_t sm_ec200_remove_file(sm_ec200_t *modem, const char *file_name)
{
    sm_modem_t *p_modem = (sm_modem_t*) modem;
    char command[128] = {0};
    sprintf(command, "AT+QFDEL=\"%s\"\r\n", file_name);
    return sm_modem_send_cmd(p_modem, command, "OK\r\n", "ERROR\r\n", 5000);
}



