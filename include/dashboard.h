#ifndef __DASHBOARD_H_
#define __DASHBOARD_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Define --------------------------------------------------------------------*/
// SSID and password for AP Mode
#define DEFAULT_AP_SSID "LORA_TESTING"
#define DEFAULT_AP_PASSWORD "12345678"

// SSID and password for STA Mode
#define DEFAULT_STA_SSID "BKIT_LUGIA_CS2"
#define DEFAULT_STA_PASSWORD "cselabc5c6"

// Define a structure to represent a location
struct Stations
{
    float lat;
    float lng;

    String stationName;
};

// const Stations STATIONS_50[] = {
//     {0, 0, "Không xác định"},
//     {10.773363, 106.656001, "VP1"},
//     {10.773072, 106.655213, "Vi tính Tài Phát"},
//     // {10.772997, 106.657328, "BK Computer"},
//     // {10.771192, 106.657806, "Trạm bus LTK"},
//     // {10.770616, 106.656731, "Bánh mì HN"},
//     // {10.771142, 106.653218, "Ngã 3 Lữ Gia"},
//     // {10.773117, 106.652984, "Mixue"},
//     // {10.774278, 106.654749, "Hẻm 281"},
//     // {10.774872, 106.656626, "Petrolimex"},
//     {10.772716, 106.654931, "VP2"}};

const Stations STATIONS_50[] = {
    {0, 0, "Không xác định"},
    {10.772716, 106.654931, "VP2"},
    {10.772604, 106.657698, "Đại học Bách Khoa"},
    {10.771294, 106.659070, "Bệnh viện Trưng Vương"},
    {10.773365, 106.661062, "Đại học Bách Khoa (cổng sau)"}
};

// LoRa HK232 testing stations GPS data for BUS_50
// const Stations STATIONS[] = {
//     {10.773365, 106.661062, "Đại học Bách Khoa (cổng sau)"},
//     {10.771294, 106.659070, "Bệnh viện Trưng Vương"},
//     {10.772604, 106.657698, "Đại học Bách Khoa"},
//     {10.776469, 106.656531, "Bưu Điện Phú Thọ"},
//     {10.778741, 106.655909, "Ngã ba Thành Thái"},
//     {10.781072, 106.655288, "Siêu thị Nguyễn Kim - CMC Tân Bình"},
//     {10.784002, 106.654430, "Cây xăng Đôi"},
//     {10.787758, 106.653368, "Chợ Tân Bình"}};

#define STATIONS_N sizeof(STATIONS) / sizeof(STATIONS[0])

enum BusDirection
{
    NOT_KNOWN,
    START_TO_END,
    END_TO_START
};

struct dirDisplay
{
    u8_t dirValue;
    String dirName;
};

const dirDisplay direction[] = {
    {NOT_KNOWN, "Không xác định"},
    {START_TO_END, "Đầu đến cuối"},
    {END_TO_START, "Cuối đến đầu"}};

void wifi_init(void);
void fs_init(void);
void websocket_init(void);
void dashboard_init(void);
String getBusInfo();
void reconnectWebServer();

#endif // DASHBOARD_H
