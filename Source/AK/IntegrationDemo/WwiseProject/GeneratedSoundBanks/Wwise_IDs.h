/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Audiokinetic Wwise generated include file. Do not edit.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __WWISE_IDS_H__
#define __WWISE_IDS_H__

#include <AK/SoundEngine/Common/AkTypes.h>

namespace AK
{
    namespace EVENTS
    {
        static const AkUniqueID DISABLE_MICROPHONE_DELAY = 6251382U;
        static const AkUniqueID DOORSLIDING = 679837841U;
        static const AkUniqueID ENABLE_MICROPHONE_DELAY = 3533161767U;
        static const AkUniqueID GUNFIRE = 2975448509U;
        static const AkUniqueID IM_1_ONE_ENEMY_WANTS_TO_FIGHT = 2221704914U;
        static const AkUniqueID IM_2_TWO_ENEMIES_WANT_TO_FIGHT = 3753105098U;
        static const AkUniqueID IM_3_SURRONDED_BY_ENEMIES = 1350929071U;
        static const AkUniqueID IM_4_DEATH_IS_COMING = 3175089270U;
        static const AkUniqueID IM_COMMUNICATION_BEGIN = 2160840676U;
        static const AkUniqueID IM_EXPLORE = 3280047539U;
        static const AkUniqueID IM_GAMEOVER = 3455955770U;
        static const AkUniqueID IM_START = 3952084898U;
        static const AkUniqueID IM_THEYAREHOSTILE = 2841817544U;
        static const AkUniqueID IM_WINTHEFIGHT = 1133905385U;
        static const AkUniqueID METRONOME_POSTMIDI = 2710399919U;
        static const AkUniqueID PAUSE_ALL = 3864097025U;
        static const AkUniqueID PAUSE_ALL_GLOBAL = 3493516265U;
        static const AkUniqueID PLAY_3DBUS_DEMO = 834165051U;
        static const AkUniqueID PLAY_CHIRP = 3187155090U;
        static const AkUniqueID PLAY_CLUSTER = 2148126352U;
        static const AkUniqueID PLAY_ENGINE = 639345804U;
        static const AkUniqueID PLAY_FOOTSTEP = 1602358412U;
        static const AkUniqueID PLAY_FOOTSTEPS = 3854155799U;
        static const AkUniqueID PLAY_HELLO = 2952797154U;
        static const AkUniqueID PLAY_HELLO_RSX = 330151568U;
        static const AkUniqueID PLAY_MARKERS_TEST = 3368417626U;
        static const AkUniqueID PLAY_MICROPHONE = 1324678662U;
        static const AkUniqueID PLAY_NONRECORDABLEMUSIC = 3873244457U;
        static const AkUniqueID PLAY_POSITIONING_DEMO = 1237313597U;
        static const AkUniqueID PLAY_RECORDABLEMUSIC = 2567011622U;
        static const AkUniqueID PLAY_ROOM_EMITTER = 2172342284U;
        static const AkUniqueID PLAY_THREE_NUMBERS_IN_A_ROW = 4142087708U;
        static const AkUniqueID PLAYMUSICDEMO1 = 519773714U;
        static const AkUniqueID PLAYMUSICDEMO2 = 519773713U;
        static const AkUniqueID PLAYMUSICDEMO3 = 519773712U;
        static const AkUniqueID RESUME_ALL = 3679762312U;
        static const AkUniqueID RESUME_ALL_GLOBAL = 1327221850U;
        static const AkUniqueID STOP_3DBUS_DEMO = 246841725U;
        static const AkUniqueID STOP_ALL = 452547817U;
        static const AkUniqueID STOP_CLUSTER = 2775363470U;
        static const AkUniqueID STOP_ENGINE = 37214798U;
        static const AkUniqueID STOP_MICROPHONE = 3629954576U;
    } // namespace EVENTS

    namespace DIALOGUE_EVENTS
    {
        static const AkUniqueID OBJECTIVE_STATUS = 3970659059U;
        static const AkUniqueID UNIT_UNDER_ATTACK = 3585983975U;
        static const AkUniqueID WALKIETALKIE = 4110439188U;
    } // namespace DIALOGUE_EVENTS

    namespace STATES
    {
        namespace HOSTILE
        {
            static const AkUniqueID GROUP = 3712907969U;

            namespace STATE
            {
                static const AkUniqueID BUM = 714721627U;
                static const AkUniqueID GANG = 685704824U;
            } // namespace STATE
        } // namespace HOSTILE

        namespace LOCATION
        {
            static const AkUniqueID GROUP = 1176052424U;

            namespace STATE
            {
                static const AkUniqueID ALLEY = 672587556U;
                static const AkUniqueID HANGAR = 2192450996U;
                static const AkUniqueID STREET = 4142189312U;
            } // namespace STATE
        } // namespace LOCATION

        namespace MUSIC
        {
            static const AkUniqueID GROUP = 3991942870U;

            namespace STATE
            {
                static const AkUniqueID EXPLORING = 1823678183U;
                static const AkUniqueID FIGHT = 514064485U;
                static const AkUniqueID FIGHT_DAMAGED = 886139701U;
                static const AkUniqueID FIGHT_DYING = 4222988787U;
                static const AkUniqueID FIGHT_LOWHEALTH = 1420167880U;
                static const AkUniqueID GAMEOVER = 4158285989U;
                static const AkUniqueID PLAYING = 1852808225U;
                static const AkUniqueID WINNING_THEFIGHT = 1323211483U;
            } // namespace STATE
        } // namespace MUSIC

        namespace OBJECTIVE
        {
            static const AkUniqueID GROUP = 6899006U;

            namespace STATE
            {
                static const AkUniqueID DEFUSEBOMB = 3261872615U;
                static const AkUniqueID NEUTRALIZEHOSTILE = 141419130U;
                static const AkUniqueID RESCUEHOSTAGE = 3841112373U;
            } // namespace STATE
        } // namespace OBJECTIVE

        namespace OBJECTIVESTATUS
        {
            static const AkUniqueID GROUP = 3299963692U;

            namespace STATE
            {
                static const AkUniqueID COMPLETED = 94054856U;
                static const AkUniqueID FAILED = 1655200910U;
            } // namespace STATE
        } // namespace OBJECTIVESTATUS

        namespace PLAYERHEALTH
        {
            static const AkUniqueID GROUP = 151362964U;

            namespace STATE
            {
                static const AkUniqueID BLASTED = 868398962U;
                static const AkUniqueID NORMAL = 1160234136U;
            } // namespace STATE
        } // namespace PLAYERHEALTH

        namespace UNIT
        {
            static const AkUniqueID GROUP = 1304109583U;

            namespace STATE
            {
                static const AkUniqueID UNIT_A = 3004848135U;
                static const AkUniqueID UNIT_B = 3004848132U;
            } // namespace STATE
        } // namespace UNIT

        namespace WALKIETALKIE
        {
            static const AkUniqueID GROUP = 4110439188U;

            namespace STATE
            {
                static const AkUniqueID COMM_IN = 1856010785U;
                static const AkUniqueID COMM_OUT = 1553720736U;
            } // namespace STATE
        } // namespace WALKIETALKIE

    } // namespace STATES

    namespace SWITCHES
    {
        namespace FOOTSTEP_GAIT
        {
            static const AkUniqueID GROUP = 4202554577U;

            namespace SWITCH
            {
                static const AkUniqueID RUN = 712161704U;
                static const AkUniqueID WALK = 2108779966U;
            } // namespace SWITCH
        } // namespace FOOTSTEP_GAIT

        namespace FOOTSTEP_WEIGHT
        {
            static const AkUniqueID GROUP = 246300162U;

            namespace SWITCH
            {
                static const AkUniqueID HEAVY = 2732489590U;
                static const AkUniqueID LIGHT = 1935470627U;
            } // namespace SWITCH
        } // namespace FOOTSTEP_WEIGHT

        namespace SURFACE
        {
            static const AkUniqueID GROUP = 1834394558U;

            namespace SWITCH
            {
                static const AkUniqueID DIRT = 2195636714U;
                static const AkUniqueID GRAVEL = 2185786256U;
                static const AkUniqueID METAL = 2473969246U;
                static const AkUniqueID WOOD = 2058049674U;
            } // namespace SWITCH
        } // namespace SURFACE

    } // namespace SWITCHES

    namespace GAME_PARAMETERS
    {
        static const AkUniqueID ENABLE_EFFECT = 2451442924U;
        static const AkUniqueID FOOTSTEP_SPEED = 3182548923U;
        static const AkUniqueID FOOTSTEP_WEIGHT = 246300162U;
        static const AkUniqueID RPM = 796049864U;
    } // namespace GAME_PARAMETERS

    namespace BANKS
    {
        static const AkUniqueID INIT = 1355168291U;
        static const AkUniqueID BGM = 412724365U;
        static const AkUniqueID BUS3D_DEMO = 3682547786U;
        static const AkUniqueID CAR = 983016381U;
        static const AkUniqueID DIRT = 2195636714U;
        static const AkUniqueID DYNAMICDIALOGUE = 1028808198U;
        static const AkUniqueID EXTERNALSOURCES = 480966290U;
        static const AkUniqueID GRAVEL = 2185786256U;
        static const AkUniqueID HUMAN = 3887404748U;
        static const AkUniqueID INTERACTIVEMUSIC = 2279279248U;
        static const AkUniqueID MARKERTEST = 2309453583U;
        static const AkUniqueID METAL = 2473969246U;
        static const AkUniqueID METRONOME = 3537469747U;
        static const AkUniqueID MICROPHONE = 2872041301U;
        static const AkUniqueID MOTION = 2012559111U;
        static const AkUniqueID MUSICCALLBACKS = 4146461094U;
        static const AkUniqueID POSITIONING_DEMO = 418215934U;
        static const AkUniqueID TESTRSX = 1868882208U;
        static const AkUniqueID WOOD = 2058049674U;
    } // namespace BANKS

    namespace BUSSES
    {
        static const AkUniqueID _3D_BUS_DEMO = 4083517055U;
        static const AkUniqueID _3D_SUBMIX_BUS = 1101487118U;
        static const AkUniqueID DRY_PATH = 1673180298U;
        static const AkUniqueID ENVIRONMENTAL_BUS = 3600197733U;
        static const AkUniqueID GAME_PAD_BUS = 3596053402U;
        static const AkUniqueID MASTER_AUDIO_BUS = 3803692087U;
        static const AkUniqueID MOTION_BUS = 4045904933U;
        static const AkUniqueID MUSIC = 3991942870U;
        static const AkUniqueID MUTED_FOR_USER_MUSIC = 1949198961U;
        static const AkUniqueID NON_RECORDABLE_BUS = 461496087U;
        static const AkUniqueID PORTALS = 2017263062U;
        static const AkUniqueID VOICES = 3313685232U;
        static const AkUniqueID VOICES_RADIO = 197057172U;
        static const AkUniqueID WET_PATH_3D = 2281484271U;
        static const AkUniqueID WET_PATH_OMNI = 1410202225U;
    } // namespace BUSSES

    namespace AUX_BUSSES
    {
        static const AkUniqueID HANGAR_ENV = 2112490296U;
        static const AkUniqueID LISTENERENV = 924456902U;
        static const AkUniqueID MASTER_MOTION_BUS_AUX = 3616209736U;
        static const AkUniqueID OUTSIDE = 438105790U;
        static const AkUniqueID ROOM = 2077253480U;
        static const AkUniqueID ROOM1 = 1359360137U;
        static const AkUniqueID ROOM2 = 1359360138U;
    } // namespace AUX_BUSSES

    namespace AUDIO_DEVICES
    {
        static const AkUniqueID COMMUNICATION_OUTPUT = 3884583641U;
        static const AkUniqueID CONTROLLER_HEADPHONES = 2868300805U;
        static const AkUniqueID DVR_BYPASS = 1535232814U;
        static const AkUniqueID NO_OUTPUT = 2317455096U;
        static const AkUniqueID PAD_OUTPUT = 666305828U;
        static const AkUniqueID SYSTEM = 3859886410U;
        static const AkUniqueID WWISE_MOTION = 1156359885U;
    } // namespace AUDIO_DEVICES

    namespace EXTERNAL_SOURCES
    {
        static const AkUniqueID EXTERN_2ND_NUMBER = 293435250U;
        static const AkUniqueID EXTERN_3RD_NUMBER = 978954801U;
        static const AkUniqueID EXTERN_1ST_NUMBER = 4004957102U;
    } // namespace EXTERNAL_SOURCES

}// namespace AK

#endif // __WWISE_IDS_H__
