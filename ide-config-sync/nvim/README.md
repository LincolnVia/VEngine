# Volt Neovim config

A focused Neovim setup for C, C++, Rust, and frontend development.

## Everyday mappings

- `<Space>` is the leader key.
- `-` opens the file browser.
- `<Space>ff` finds files; `<Space>fg` searches project text.
- `<Space>f` formats; saving formats automatically.
- `gd`, `gr`, and `gI` navigate definitions, references, and implementations.
- `<Space>le` opens workspace diagnostics, including clangd/compiler errors.
- `<Space>la`, `<Space>lr`, and `<Space>ld` run code actions, rename, and show diagnostics.
- `Ctrl-h/j/k/l` moves between windows on a QWERTY keyboard.
- `F5`, `F10`, `F11`, and `F12` control the debugger.
- `<Space>db` toggles a breakpoint; `<Space>du` toggles the debug UI.
- `<Space>lg` adds a get/set property for the C# field under the cursor; private fields get a public property and remain as backing fields.
- `<Space>uc` creates a MonoBehaviour script; `<Space>us` creates a ScriptableObject;
  `<Space>ue` creates an EditorWindow; `<Space>uf` finds C# scripts.
  Script generators use the selected NvimTree folder, or the selected file's
  folder; if NvimTree has no selection, they use Neovim's current directory.

Run `:Mason` to inspect language tools, `:Lazy` to inspect plugins, and
`:checkhealth` when troubleshooting.
