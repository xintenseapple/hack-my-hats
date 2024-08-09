from __future__ import annotations

from typing import Any, Dict

from typing_extensions import Self, final, override

from tophat.api.hat import HackableHat


@final
class HeapMinerHat(HackableHat):

    @property
    @override
    def image_name(self: Self) -> str:
        return 'tophat:heap_miner'

    @property
    @override
    def extra_docker_args(self: Self) -> Dict[str, Any]:
        return {
            'ports': {'1234/tcp': 1234}
        }
