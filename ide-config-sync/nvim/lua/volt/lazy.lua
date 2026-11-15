local lazypath = vim.fn.stdpath("data") .. "/lazy/lazy.nvim"

if not vim.uv.fs_stat(lazypath) then
	local output = vim.fn.system({
		"git",
		"clone",
		"--filter=blob:none",
		"--branch=stable",
		"https://github.com/folke/lazy.nvim.git",
		lazypath,
	})
	if vim.v.shell_error ~= 0 then
		error("Unable to install lazy.nvim:\n" .. output)
	end
end

vim.opt.rtp:prepend(lazypath)

require("lazy").setup({
	spec = { { import = "volt.plugins" } },
	install = { colorscheme = { "catppuccin", "habamax" } },
	checker = { enabled = true, notify = false },
	change_detection = { notify = false },
	performance = {
		rtp = { disabled_plugins = { "gzip", "tarPlugin", "tohtml", "tutor", "zipPlugin" } },
	},
	{
		"nvim-mini/mini.move",
		version = false,
		config = function()
			require("mini.move").setup()
		end,
	},
})
require("nvim-tree").setup({
	on_attach = function(bufnr)
		local api = require("nvim-tree.api")

		-- Load the default keymaps first
		api.config.mappings.default_on_attach(bufnr)

		-- Custom mapping: Pressing 'C' or your choice will set the folder as root
		-- api.tree.change_root_to_node changes the tree root to the folder under the cursor
		vim.keymap.set("n", "C", api.tree.change_root_to_node, {
			buffer = bufnr,
			noremap = true,
			silent = true,
			desc = "Set selected folder as root",
		})
	end,
})
