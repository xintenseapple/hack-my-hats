from __future__ import annotations

from typing_extensions import Self, final, override

from tophat.api.hat import HackableHat


@final
class NFCWranglerHat(HackableHat):

    @property
    @override
    def image_name(self: Self) -> str:
        return 'tophat:nfc_wrangler'
