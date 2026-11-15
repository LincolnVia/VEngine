return {
  {
    "mfussenegger/nvim-dap",
    keys = {
      { "<F5>", function() require("dap").continue() end, desc = "Debug: start/continue" },
      { "<F10>", function() require("dap").step_over() end, desc = "Debug: step over" },
      { "<F11>", function() require("dap").step_into() end, desc = "Debug: step into" },
      { "<F12>", function() require("dap").step_out() end, desc = "Debug: step out" },
      { "<leader>db", function() require("dap").toggle_breakpoint() end, desc = "Toggle breakpoint" },
      { "<leader>dr", function() require("dap").repl.open() end, desc = "Open debug REPL" },
      { "<leader>du", function() require("dapui").toggle() end, desc = "Toggle debug UI" },
    },
    dependencies = {
      "rcarriga/nvim-dap-ui",
      "nvim-neotest/nvim-nio",
    },
    config = function()
      local dap = require("dap")
      local dapui = require("dapui")

      dapui.setup()
      dap.listeners.before.attach.dapui_config = dapui.open
      dap.listeners.before.launch.dapui_config = dapui.open
      dap.listeners.before.event_terminated.dapui_config = dapui.close
      dap.listeners.before.event_exited.dapui_config = dapui.close

      dap.adapters.codelldb = { type = "executable", command = "codelldb" }
      dap.configurations.cpp = {
        {
          name = "Launch executable",
          type = "codelldb",
          request = "launch",
          program = function()
            return vim.fn.input("Executable: ", vim.fn.getcwd() .. "/", "file")
          end,
          cwd = "${workspaceFolder}",
          stopOnEntry = false,
        },
      }
      dap.configurations.c = dap.configurations.cpp
      dap.configurations.rust = dap.configurations.cpp
    end,
  },
}
