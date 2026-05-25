import json
import boto3
import logging
import ask_sdk_core.utils as ask_utils
from ask_sdk_core.skill_builder import SkillBuilder
from ask_sdk_core.dispatch_components import AbstractRequestHandler, AbstractExceptionHandler

logger = logging.getLogger()
logger.setLevel(logging.INFO)

REGION   = "us-east-1"
REPROMPT = "¿Qué más quieres saber del ventilador?"


dynamodb = boto3.resource("dynamodb", region_name=REGION)
TABLE    = dynamodb.Table("user_thing")   

def get_thing_name(handler_input) -> str:
    """
    Lee el thing_name asociado al usuario en DynamoDB.
    La tabla 'user_thing' tiene:
        PK  user_id   (string)  → Alexa user ID
        ATT thing_name (string) → nombre del Thing en IoT Core
    """
    user_id = handler_input.request_envelope.session.user.user_id
    response = TABLE.get_item(Key={"user_id": user_id})
    item = response.get("Item")
    if not item:
        raise ValueError(f"No se encontró thing_name para user_id={user_id}")
    return item["thing_name"]


def get_iot_data_client():
    iot = boto3.client("iot", region_name=REGION)
    endpoint = iot.describe_endpoint(endpointType="iot:Data-ATS")["endpointAddress"]
    return boto3.client(
        "iot-data",
        region_name=REGION,
        endpoint_url=f"https://{endpoint}",
    )


def get_shadow_state(thing_name: str) -> dict:
    """Retorna el payload completo del shadow."""
    client = get_iot_data_client()
    response = client.get_thing_shadow(thingName=thing_name)
    return json.loads(response["payload"].read())

def get_shadow_variable(thing_name: str, variable: str,
                        section: str = "reported", default=None):
    """Extrae una variable específica del shadow."""
    try:
        payload = get_shadow_state(thing_name)
        return payload.get("state", {}).get(section, {}).get(variable, default)
    except Exception as e:
        logger.error(f"Error obteniendo {variable} de {section}: {e}")
        return default



def update_shadow_desired(thing_name: str, desired: dict) -> bool:
    """
    Escribe cualquier combinación de campos en desired.
    Ejemplo: update_shadow_desired(thing, {"speed": 70, "autoMode": True})
    """
    try:
        client = get_iot_data_client()
        client.update_thing_shadow(
            thingName=thing_name,
            payload=json.dumps({"state": {"desired": desired}})
        )
        return True
    except Exception as e:
        logger.error(f"Error actualizando shadow desired {desired}: {e}")
        return False



def speak(handler_input, text: str, keep_session: bool = True):
    """Atajo para construir respuesta con o sin sesión abierta."""
    rb = handler_input.response_builder.speak(text)
    if keep_session:
        rb = rb.ask(REPROMPT)
    return rb.response



class LaunchRequestHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_request_type("LaunchRequest")(handler_input)

    def handle(self, handler_input):
        text = (
            "Bienvenido al control del ventilador. "
            "Puedes preguntarme la temperatura, la humedad, la velocidad, "
            "el estado completo, activar o desactivar el modo automático, "
            "cambiar el umbral de temperatura, o cambiar la velocidad."
        )
        return speak(handler_input, text)



class HelloIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("HelloIntent")(handler_input)

    def handle(self, handler_input):
        return speak(handler_input,
                     "Hola! Estoy aquí para ayudarte a controlar tu ventilador.")



class HelpIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("AMAZON.HelpIntent")(handler_input)

    def handle(self, handler_input):
        text = (
            "Puedes decirme: temperatura, humedad, velocidad actual, estado completo, "
            "enciende el ventilador, apaga el ventilador, "
            "activa o desactiva el modo automático, "
            "cambia el umbral a un número, "
            "o cambia la velocidad a un número entre cero y cien."
        )
        return speak(handler_input, text)



class GetTemperatureIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("GetTemperatureIntent")(handler_input)

    def handle(self, handler_input):
        try:
            thing = get_thing_name(handler_input)
            temp  = get_shadow_variable(thing, "temperature")
            text  = (f"La temperatura actual es de {temp} grados Celsius."
                     if temp is not None
                     else "No pude obtener la temperatura, el sensor no está disponible.")
        except Exception as e:
            logger.error(e)
            text = "No pude identificar tu ventilador. Verifica tu cuenta."
        return speak(handler_input, text)

class GetHumidityIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("GetHumidityIntent")(handler_input)

    def handle(self, handler_input):
        try:
            thing    = get_thing_name(handler_input)
            humidity = get_shadow_variable(thing, "humidity")
            text     = (f"La humedad actual es del {humidity} por ciento."
                        if humidity is not None
                        else "No pude obtener la humedad, el sensor no está disponible.")
        except Exception as e:
            logger.error(e)
            text = "No pude identificar tu ventilador. Verifica tu cuenta."
        return speak(handler_input, text)


class GetSpeedLevelIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("GetSpeedLevelIntent")(handler_input)

    def handle(self, handler_input):
        try:
            thing = get_thing_name(handler_input)
            speed = get_shadow_variable(thing, "speed")
            if speed is None:
                text = "No pude obtener la velocidad del ventilador."
            elif speed == 0:
                text = "El ventilador está apagado, su velocidad es cero."
            else:
                text = f"La velocidad actual del ventilador es {speed}."
        except Exception as e:
            logger.error(e)
            text = "No pude identificar tu ventilador. Verifica tu cuenta."
        return speak(handler_input, text)


class GetAllValuesIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("GetAllValuesIntent")(handler_input)

    def handle(self, handler_input):
        try:
            thing    = get_thing_name(handler_input)
            payload  = get_shadow_state(thing)
            reported = payload.get("state", {}).get("reported", {})

            speed     = reported.get("speed")
            temp      = reported.get("temperature")
            humidity  = reported.get("humidity")
            auto_mode = reported.get("autoMode")
            threshold = reported.get("tempThreshold")

            if any(v is None for v in [speed, temp]):
                text = "No pude obtener el estado completo del ventilador."
            else:
                estado   = "apagado" if speed == 0 else f"encendido a velocidad {speed}"
                auto_str = "activado" if auto_mode else "desactivado"
                hum_str  = f"{humidity} por ciento" if humidity is not None else "no disponible"
                thr_str  = f"{threshold} grados" if threshold is not None else "no configurado"

                text = (
                    f"Estado completo del ventilador: "
                    f"el ventilador está {estado}. "
                    f"Temperatura {temp} grados Celsius. "
                    f"Humedad {hum_str}. "
                    f"Modo automático {auto_str}. "
                    f"Umbral de temperatura {thr_str}."
                )
        except Exception as e:
            logger.error(f"GetAllValuesIntent error: {e}")
            text = "Hubo un error al obtener el estado completo."
        return speak(handler_input, text)


class UpdateSpeedLevelIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("UpdateSpeedLevelIntent")(handler_input)

    def handle(self, handler_input):
        slots     = handler_input.request_envelope.request.intent.slots
        raw_speed = slots.get("speed") and slots["speed"].value

        if raw_speed is None:
            return speak(handler_input,
                         "¿A qué velocidad quieres poner el ventilador? "
                         "Di un número entre cero y cien.")
        try:
            speed = int(float(raw_speed))
        except (ValueError, TypeError):
            return speak(handler_input,
                         "No entendí el número. Di una velocidad entre cero y cien.")

        if not (0 <= speed <= 100):
            return speak(handler_input,
                         f"{speed} está fuera de rango. "
                         "La velocidad debe ser entre cero y cien.")
        try:
            thing = get_thing_name(handler_input)
            ok    = update_shadow_desired(thing, {"speed": speed})
            if ok:
                text = ("El ventilador ha sido apagado."
                        if speed == 0
                        else f"Velocidad actualizada a {speed}.")
            else:
                text = "No pude actualizar la velocidad. Inténtalo de nuevo."
        except Exception as e:
            logger.error(e)
            text = "No pude identificar tu ventilador. Verifica tu cuenta."
        return speak(handler_input, text)



class SetAutoModeIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("SetAutoModeIntent")(handler_input)

    def handle(self, handler_input):
        intent = handler_input.request_envelope.request.intent
        slots = intent.slots if intent.slots else {}

        action = (
            slots.get("autoAction").value
            if slots.get("autoAction")
            else None
        )

        if action is None:
            return speak(handler_input,
                         "¿Quieres activar o desactivar el modo automático?")

        action_lower = action.lower()
        if any(w in action_lower for w in ["activ", "encend", "on"]):
            auto_mode = True
            confirm   = "Modo automático activado. El ventilador se encenderá automáticamente cuando se supere el umbral de temperatura."
        elif any(w in action_lower for w in ["desactiv", "apag", "off"]):
            auto_mode = False
            confirm   = "Modo automático desactivado."
        else:
            return speak(handler_input,
                         "No entendí. Di activar o desactivar el modo automático.")
        try:
            thing = get_thing_name(handler_input)
            ok    = update_shadow_desired(thing, {"autoMode": auto_mode})
            text  = confirm if ok else "No pude cambiar el modo automático. Inténtalo de nuevo."
        except Exception as e:
            logger.error(e)
            text = "No pude identificar tu ventilador. Verifica tu cuenta."
        return speak(handler_input, text)


class GetAutoModeIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("GetAutoModeIntent")(handler_input)

    def handle(self, handler_input):
        try:
            thing     = get_thing_name(handler_input)
            auto_mode = get_shadow_variable(thing, "autoMode")
            threshold = get_shadow_variable(thing, "tempThreshold")

            if auto_mode is None:
                text = "No pude obtener el estado del modo automático."
            else:
                estado = "activado" if auto_mode else "desactivado"
                thr_str = (f" El umbral configurado es de {threshold} grados."
                           if threshold is not None else "")
                text = f"El modo automático está {estado}.{thr_str}"
        except Exception as e:
            logger.error(e)
            text = "No pude identificar tu ventilador. Verifica tu cuenta."
        return speak(handler_input, text)


class SetTempThresholdIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("SetTempThresholdIntent")(handler_input)

    def handle(self, handler_input):
        slots       = handler_input.request_envelope.request.intent.slots
        raw_thresh  = slots.get("threshold") and slots["threshold"].value

        if raw_thresh is None:
            return speak(handler_input,
                         "¿A qué temperatura quieres que se active el ventilador automáticamente? "
                         "Di un número en grados Celsius.")
        try:
            threshold = int(float(raw_thresh))
        except (ValueError, TypeError):
            return speak(handler_input,
                         "No entendí el número. Di la temperatura en grados Celsius.")

        if not (0 <= threshold <= 60):
            return speak(handler_input,
                         f"{threshold} grados está fuera de rango. "
                         "El umbral debe estar entre 0 y 60 grados.")
        try:
            thing = get_thing_name(handler_input)
            ok    = update_shadow_desired(thing, {"tempThreshold": threshold})
            text  = (f"Umbral de temperatura actualizado a {threshold} grados. "
                     f"El ventilador se activará automáticamente si la temperatura lo supera."
                     if ok
                     else "No pude actualizar el umbral. Inténtalo de nuevo.")
        except Exception as e:
            logger.error(e)
            text = "No pude identificar tu ventilador. Verifica tu cuenta."
        return speak(handler_input, text)


class GetTempThresholdIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("GetTempThresholdIntent")(handler_input)

    def handle(self, handler_input):
        try:
            thing     = get_thing_name(handler_input)
            threshold = get_shadow_variable(thing, "tempThreshold")
            text      = (f"El umbral de temperatura está configurado a {threshold} grados Celsius."
                         if threshold is not None
                         else "No hay un umbral configurado.")
        except Exception as e:
            logger.error(e)
            text = "No pude identificar tu ventilador. Verifica tu cuenta."
        return speak(handler_input, text)



class CancelAndStopIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return (
            ask_utils.is_intent_name("AMAZON.CancelIntent")(handler_input)
            or ask_utils.is_intent_name("AMAZON.StopIntent")(handler_input)
        )

    def handle(self, handler_input):
        # Sin .ask() → cierra la sesión intencionalmente
        return handler_input.response_builder.speak("Hasta luego.").response


class CatchAllExceptionHandler(AbstractExceptionHandler):
    def can_handle(self, handler_input, exception):
        return True

    def handle(self, handler_input, exception):
        logger.error(f"Excepción no controlada: {exception}", exc_info=True)
        return speak(handler_input,
                     "Hubo un problema. Por favor intenta de nuevo.")


sb = SkillBuilder()

sb.add_request_handler(LaunchRequestHandler())
sb.add_request_handler(HelloIntentHandler())
sb.add_request_handler(HelpIntentHandler())
sb.add_request_handler(GetTemperatureIntentHandler())
sb.add_request_handler(GetHumidityIntentHandler())
sb.add_request_handler(GetSpeedLevelIntentHandler())
sb.add_request_handler(GetAllValuesIntentHandler())
sb.add_request_handler(UpdateSpeedLevelIntentHandler())
sb.add_request_handler(SetAutoModeIntentHandler())
sb.add_request_handler(GetAutoModeIntentHandler())
sb.add_request_handler(SetTempThresholdIntentHandler())
sb.add_request_handler(GetTempThresholdIntentHandler())
sb.add_request_handler(CancelAndStopIntentHandler())
sb.add_exception_handler(CatchAllExceptionHandler())

lambda_handler = sb.lambda_handler()