return {
	{
		"nvim-tree/nvim-tree.lua",
		version = "*",
		lazy = false,
		dependencies = {
			"nvim-tree/nvim-web-devicons", -- Optional: adds file icons (requires Nerd Font)
		},
		config = function()
			-- Recommended: Disable netrw at the very start of your init.lua
			vim.g.loaded_netrw = 1
			vim.g.loaded_netrwPlugin = 1

			-- Enable 24-bit color
			vim.opt.termguicolors = true

			require("nvim-tree").setup({
				sort = {
					sorter = "case_sensitive",
				},
				view = {
					width = 30,
					side = "left", -- Places sidebar on the left
				},
				renderer = {
					group_empty = true,
				},
				filters = {
					dotfiles = false, -- Change to true to hide hidden files
				},
			})

			-- Keybindings
			vim.keymap.set("n", "<leader>e", "<cmd>NvimTreeToggle<cr>", { desc = "Toggle File Tree" })
			vim.keymap.set("n", "<leader>ef", "<cmd>NvimTreeFindFile<cr>", { desc = "Find Current File in Tree" })
		end,
	},
}
