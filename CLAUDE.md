# CLAUDE.md

Guidance for Claude Code when working in this repository.

## Project

**modbus-simulated-dataset** — generates a simulated Modbus (industrial control
system / SCADA) network traffic dataset, intended for ICS security research,
anomaly detection, and ML model training.

> The repository is currently empty. Fill in the sections below as the codebase
> takes shape, and remove this note once they reflect real structure.

## Layout

_To be documented as the project grows. Suggested structure:_

- `src/` — dataset generation and Modbus simulation code
- `data/` — generated datasets (gitignored)
- `tests/` — test suite

## Commands

_Add the real commands once tooling is chosen, e.g.:_

```bash
# Setup
python3 -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt

# Generate the dataset
# python3 -m src.generate ...

# Tests
pytest
```

## Conventions

- Keep generated data out of git (see `.gitignore`).
- Document the schema/columns of any produced dataset.
- Never commit secrets or real network captures with sensitive data.
