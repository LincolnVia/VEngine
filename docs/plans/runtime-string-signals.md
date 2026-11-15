# Runtime string signals for entities

## Summary

Add a string-based signal path so an `info_trigger` can notify an `info_event` at runtime. Each runtime entity has a `listen` string and an `OnSignalReceived` signal. A sender's `target` string is delivered to entities whose `listen` value matches it.

## Implementation changes

- Add `listen` and `OnSignalReceived` to the `RuntimeEntity` base so every runtime entity can receive signals.
- Add a `World` signal dispatch method that checks the live runtime entities and emits `OnSignalReceived` for matching listeners.
- Change `info_trigger`'s `target` from an integer to a string and dispatch it when the trigger fires.
- Add and register `info_event`. It listens for its configured `listen` string, then dispatches its own `target` string so events can be chained. An empty target ends the chain.
- Keep `info_interactable`'s existing integer target behavior separate. Use the map's `target` and `listen` keys for this signal flow; leave the existing boolean `EventListener` state API unchanged.

## Test plan

- Confirm a trigger fires its matching `info_event` at runtime.
- Confirm a nonmatching `listen` value receives nothing.
- Confirm one `info_event` can relay to another.
- Confirm an empty target stops the chain and level reload does not leave stale listeners.

## Assumptions

- Signal delivery is synchronous during runtime dispatch.
- Use the correctly spelled public name `OnSignalReceived`.
