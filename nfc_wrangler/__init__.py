from __future__ import annotations

from typing_extensions import Self, final

from tophat.api.hat import HackableHat


@final
class NFCWranglerHat(HackableHat):

    @property
    def image_name(self: Self) -> str:
        return 'tophat:nfc_wrangler'
