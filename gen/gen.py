#!/usr/bin/python3.9
from typing import Any
import io
import yaml
import re
import jinja2
import codecs

# Load file
stream: io.TextIOWrapper = open("./gen/lex.yaml", "r")
document: Any = yaml.safe_load(stream)

# Helper
def escape(value: str) -> str:
    return codecs.getencoder("unicode_escape")(value)[0]

# Common


class State:
    def __init__(self) -> None:
        self.name = None
        self.is_final = False
        self.can_continue = False
        self.default_transition = None
        self.transitions = dict()
        self.token = None
        pass


class ProcessedState:
    name: str
    token: str
    is_final: bool
    can_continue: bool
    transitions: list[int]

    def __init__(self) -> None:
        self.name = "unknown"
        self.token = None
        self.is_final = False
        self.can_continue = False
        self.transitions = list()
        pass


states: dict[str, State] = dict()

# Read lex yaml file


def parse_state(state_name: str, state_yaml: dict):
    if state_name in states:
        print(f"Error: State {state_name} already exists.")

    state: State = State()
    state.name = state_name

    for key, value in state_yaml.items():
        if len(key) == 1:
            if type(value) == str or value is None:
                state.transitions[key[0]] = value
            elif type(value) == dict:
                state.transitions[key[0]] = key
                parse_state(key, value)
            else:
                print(
                    f"Error: Invalid transition {escape(key)} in state {state_name}. Expected string.")
        elif key == "is_final":
            if type(value) == bool:
                state.is_final = value
            else:
                print(
                    f"Error: Invalid final flag in state {state_name}. Expected boolean.")
        elif key == "can_continue":
            if type(value) == bool:
                state.can_continue = value
            else:
                print(
                    f"Error: Invalid can continue flag in state {state_name}. Expected boolean.")
        elif key == "default":
            if type(value) == str or value is None:
                state.default_transition = value
            else:
                print(
                    f"Error: Invalid default field in state {state_name}. Expected string.")
        elif key == "token":
            if type(value) == str:
                state.token = value
            else:
                print(
                    f"Error: Invalid token field in state {state_name}. Expected dict.")
        else:
            range_result = re.search(r"^(.)-(.)(?:/(.*))?$", key)
            if range_result != None:
                min = range_result.group(1)
                max = range_result.group(2)

                exclude: str = range_result.group(3)
                if exclude is None:
                    exclude = ""

                # Handle nested state
                if type(value) == dict:
                    parse_state(key, value)
                    value = key

                if type(value) == str or value is None:
                    for n in range(ord(min), ord(max) + 1):
                        transition_character = chr(n)
                        if transition_character not in exclude:
                            state.transitions[transition_character] = value
                else:
                    print(
                        f"Error: Invalid transition {escape(key)} in state {state_name}. Expected string.")
            else:
                print(f"Error: Invalid field {escape(key)} in state {state_name}.")

    states[state_name] = state
    pass


for state_name, state_yaml in document.items():
    if type(state_yaml) is dict:
        parse_state(state_name, state_yaml)
    else:
        print("Error: Invalid state. Expected object.")

#
default_state: State = State()
default_state.name = "default"
default_state.is_final = True
default_state.can_continue = False
default_state.token = None

state_list: list[State] = list()
state_list.append(default_state)

state_indices: dict[str, int] = dict()
state_indices["default"] = 0

for state_name, state in states.items():
    state_indices[state_name] = len(state_list)
    state_list.append(state)

    # Check default transition
    if state.default_transition != None and not state.default_transition in states:
        print(f"Error: State {state.default_transition} does not exist.")

    # Check transitions
    for transition_character, transition in state.transitions.items():
        if transition not in states and transition is not None:
            print(f"Error: State {transition} does not exist.")

######## Process States ########
processed_state_list: list[ProcessedState] = []
for state in state_list:
    processed_state: ProcessedState = ProcessedState()
    processed_state.name = state.name
    processed_state.token = state.token
    processed_state.is_final = state.is_final
    processed_state.can_continue = state.can_continue

    # Process transistions
    for n in range(0, 256):
        next_state = state.default_transition

        transition_character = chr(n)
        if transition_character in state.transitions:
            next_state = state.transitions[transition_character]
        
        next_state = state_indices[next_state] if next_state is not None else 0

        # Add processed transistion
        processed_state.transitions.append(next_state)

    # Add processed state
    processed_state_list.append(processed_state)

######## Generate C Code ########
environment = jinja2.Environment(loader=jinja2.FileSystemLoader("gen/templates/"))
lexer_template = environment.get_template("lexer.txt")
state_flags_template = environment.get_template("state_flags.txt")
state_name_template = environment.get_template("state_name.txt")
state_token_template = environment.get_template("state_token.txt")
state_transitions_template = environment.get_template("state_transitions.txt")

buffer = lexer_template.render(states=processed_state_list) + "\n\n"
buffer += state_transitions_template.render(states=processed_state_list) + "\n\n"
buffer += state_flags_template.render(states=processed_state_list) + "\n\n"
buffer += state_token_template.render(states=processed_state_list) + "\n\n"
# buffer += state_name_template.render(states=processed_state_list)

# Save lexer
stream: io.TextIOWrapper = open("./wasp-lang/wasp_lexer.g.h", "w")
stream.write(buffer)
stream.close()
