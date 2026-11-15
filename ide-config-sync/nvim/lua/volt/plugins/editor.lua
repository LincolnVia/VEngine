return {
  {
    "stevearc/oil.nvim",
    cmd = "Oil",
    keys = { { "-", "<cmd>Oil<CR>", desc = "Open parent directory" } },
    dependencies = { "nvim-tree/nvim-web-devicons" },
    opts = {
      default_file_explorer = true,
      view_options = { show_hidden = true },
    },
  },
  {
    "nvim-telescope/telescope.nvim",
    cmd = "Telescope",
    keys = {
      { "<leader>ff", "<cmd>Telescope find_files<CR>", desc = "Find files" },
      { "<leader>fg", "<cmd>Telescope live_grep<CR>", desc = "Find text" },
      { "<leader>fb", "<cmd>Telescope buffers<CR>", desc = "Find buffers" },
      { "<leader>fh", "<cmd>Telescope help_tags<CR>", desc = "Find help" },
      { "<leader>fr", "<cmd>Telescope oldfiles<CR>", desc = "Recent files" },
      { "<leader>gc", "<cmd>Telescope git_commits<CR>", desc = "Git commits" },
      {
        "<leader>gb",
        function()
          vim.ui.select(
            { "Branches (local + remote)", "All commits (all refs)" },
            { prompt = "Select branch or commit:" },
            function(choice)
              if choice == "Branches (local + remote)" then
                require("telescope.builtin").git_branches({
                  show_remote_tracking_branches = true,
                })
              elseif choice == "All commits (all refs)" then
                require("telescope.builtin").git_commits({
                  git_command = { "git", "log", "--all", "--pretty=oneline", "--abbrev-commit" },
                })
              end
            end
          )
        end,
        desc = "Select branch or commit",
      },
      {
        "<leader>gbc",
        function()
          vim.ui.input({ prompt = "New branch name: " }, function(name)
            if not name or name:match("^%s*$") then
              return
            end
            local output = vim.fn.system({ "git", "switch", "-c", name })
            if vim.v.shell_error ~= 0 then
              vim.notify(vim.trim(output), vim.log.levels.ERROR)
            else
              vim.notify("Created and switched to branch " .. name, vim.log.levels.INFO)
            end
          end)
        end,
        desc = "Create and switch to branch",
      },
      { "<leader>gs", "<cmd>Telescope git_status<CR>", desc = "Git status" },
    },
    dependencies = {
      "nvim-lua/plenary.nvim",
      "nvim-tree/nvim-web-devicons",
      { "nvim-telescope/telescope-fzf-native.nvim", build = "make" },
    },
    config = function()
      local telescope = require("telescope")
      telescope.setup({
        defaults = {
          layout_strategy = "horizontal",
          layout_config = { prompt_position = "top" },
          sorting_strategy = "ascending",
        },
      })
      pcall(telescope.load_extension, "fzf")
    end,
  },
  {
    "lewis6991/gitsigns.nvim",
    event = { "BufReadPre", "BufNewFile" },
    opts = {
      current_line_blame = true,
      current_line_blame_opts = { delay = 500 },
      on_attach = function(bufnr)
        local gs = package.loaded.gitsigns
        local function map(mode, lhs, rhs, desc)
          vim.keymap.set(mode, lhs, rhs, { buffer = bufnr, desc = desc })
        end
        map("n", "]h", gs.next_hunk, "Next Git hunk")
        map("n", "[h", gs.prev_hunk, "Previous Git hunk")
        map("n", "<leader>gp", gs.preview_hunk, "Preview Git hunk")
        map("n", "<leader>gr", gs.reset_hunk, "Reset Git hunk")
        map("n", "<leader>gB", gs.blame_line, "Git blame line")
      end,
    },
  },
  {
    "windwp/nvim-autopairs",
    event = "InsertEnter",
    opts = {},
  },
  {
    "numToStr/Comment.nvim",
    event = { "BufReadPost", "BufNewFile" },
    opts = {},
  },
}
